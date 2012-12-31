/* Copyright 2012 Jesse Weaver
 * 
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 * 
 *        http://www.apache.org/licenses/LICENSE-2.0
 * 
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 *    implied. See the License for the specific language governing
 *    permissions and limitations under the License.
 */

#include <deque>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <vector>
#include "rif/RIFActionBlock.h"
#include "rif/RIFCondition.h"
#include "rif/RIFRule.h"

using namespace ptr;
using namespace rif;
using namespace std;

#define DECIDE 0
#define NOPROBLEMS 1
#define NONTRIVIAL 1

#if DECIDE
#error "Decision isn't working yet.  Define DECIDE as 0."
#endif

typedef set<RIFTerm,
            bool (*)(const RIFTerm &, const RIFTerm &)>
        TSet;
#define TSET(varname) TSet varname(RIFTerm::cmplt0)

typedef map<RIFTerm,
            TSet,
            bool (*)(const RIFTerm &, const RIFTerm &)>
        NMap;
#define NMAP(varname) NMap varname(RIFTerm::cmplt0)

DPtr<uint8_t> *cstr2dptr(const char *cstr) {
  try {
    size_t len = strlen(cstr);
    DPtr<uint8_t> *p;
    NEW(p, MPtr<uint8_t>, len);
    ascii_strncpy(p->dptr(), cstr, len);
    return p;
  } RETHROW_BAD_ALLOC
}

void printstr(ostream &stream, DPtr<uint8_t> *str) {
  const uint8_t *p = str->dptr();
  const uint8_t *end = p + str->size();
  for (; p != end; ++p) {
    stream << to_lchar(*p);
  }
}

ostream &operator<<(ostream &stream, const RIFTerm &term) {
  DPtr<uint8_t> *str = term.toUTF8String();
  printstr(stream, str);
  str->drop();
  return stream;
}

ostream &operator<<(ostream &stream, const RIFAtomic &atom) {
  DPtr<uint8_t> *str = atom.toUTF8String();
  printstr(stream, str);
  str->drop();
  return stream;
}

ostream &operator<<(ostream &stream, const RIFCondition &cond) {
  DPtr<uint8_t> *str = cond.toUTF8String();
  printstr(stream, str);
  str->drop();
  return stream;
}

ostream &operator<<(ostream &stream, const RIFRule &rule) {
  DPtr<uint8_t> *str = rule.toUTF8String();
  printstr(stream, str);
  str->drop();
  return stream;
}

bool isFixed(const RIFAtomic &atom) {
  if (atom.getType() == EXTERNAL) {
    return true;
  }
  if (atom.getType() != EQUALITY) {
    return false;
  }
  if (atom.isGround()) {
    pair<RIFTerm, RIFTerm> eq = atom.getEquality();
    if (eq.first.equals(eq.second)) {
      return true;
    }
  }
  return false;
}

bool isDerivable(const RIFAtomic &atom) {
  enum RIFAtomicType type = atom.getType();
  return type == ATOM || type == FRAME || type == MEMBERSHIP;
}

const RIFConst &predListContains() {
  static const char *pred_list_contains_str = "\"http://www.w3.org/2007/rif-builtin-predicate#list-contains\"^^<http://www.w3.org/2007/rif#iri>";
  static RIFConst *pred_list_contains = NULL;
  if (pred_list_contains == NULL) {
    size_t len = strlen(pred_list_contains_str);
    DPtr<uint8_t> *p;
    NEW(p, MPtr<uint8_t>, len);
    ascii_strncpy(p->dptr(), pred_list_contains_str, len);
    RIFConst temp = RIFConst::parse(p);
    p->drop();
    NEW(pred_list_contains, RIFConst, temp);
  }
  return *pred_list_contains;
}

inline
bool isPredListContains(const RIFConst rifconst) {
  return predListContains().equals(rifconst);
}

class Pattern {
public:
  NMap neqs;
  RIFAtomic atom;
  Pattern(const RIFAtomic &a)
    : atom(a), neqs(NMap(RIFTerm::cmplt0)) {
    validate();
  }
  Pattern(const RIFAtomic &a, const NMap &b)
    : atom(a), neqs(b) {
    validate();
  }
  Pattern(const Pattern &p)
    : atom(p.atom), neqs(p.neqs) {
    // do nothing
  }
  ~Pattern() {
    // do nothing
  }
  void validate() const {
    size_t arity = this->atom.getArity();
    if (this->atom.getType() == FRAME && arity != 3) {
      cerr << "[ERROR] Frame formula " << this->atom << " must have exactly one slot." << endl;
      THROW(TraceableException, "All frame formulas must have exactly one slot.");
    }
    if (this->atom.getType() == EXTERNAL) {
      cerr << "[WARNING] Presence of builtin in " << this->atom << " means that reduction to SAT is inexact.  Specifically, when the SAT formula is satisfied, then the distribution will work,  but if the SAT formula is not satisfied, then the distribution may or may not work (depending on nature and usage of the builtin)." << endl;
    }
    size_t i;
    for (i = this->atom.startPart(); i <= arity; ++i) {
      RIFTerm term = this->atom.getPart(i);
      if (term.containsFunction()) {
        cerr << "[WARNING] Presence of function in " << term << " means that reduction to SAT is inexact.  Specifically, when the SAT formula is satisfied, then the distribution will work,  but if the SAT formula is not satisfied, then the distribution may or may not work (depending on nature and usage of the function)." << endl;
        if (term.isGround()) {
          cerr << "[ERROR] Must replace ground function in " << term << " with constant." << endl;
          THROW(TraceableException, "Must replace ground function with constant.");
        }
      }
    }
    NMap::const_iterator mit = this->neqs.begin();
    for (; mit != this->neqs.end(); ++mit) {
      if (mit->first.getType() != VARIABLE && mit->first.getType() != FUNCTION) {
        cerr << "[ERROR] Found key " << mit->first << " in NMap.  Should be a variable or  non-ground function." << endl;
        THROW(TraceableException, "Keys in NMap must be variables or (non-ground) functions.");
      }
      TSet::const_iterator it = mit->second.begin();
      for (; it != mit->second.end(); ++it) {
        if (it->getType() != CONSTANT && it->getType() != LIST) {
          cerr << "[ERROR] Found value " << *it << " in NMap.  Should be constant or list." << endl;
          THROW(TraceableException, "Values in NMap (or rather in the TSets of NMap) must be constants or lists.");
        }
      }
    }
  }
  static bool implies(const RIFTerm &t1, const TSet *n1,
                      const RIFTerm &t2, const TSet *n2) {
    if (t1.isGround()) {
      if (t2.isGround()) {
        return t1.equals(t2);
      }
      return n2 == NULL || n2->count(t1) <= 0;
    }
    if (t2.isGround()) {
      return false;
    }
    if (n2 == NULL) {
      return true;
    }
    if (n1 == NULL) {
      return n2->empty();
    }
    TSet::const_iterator n2it = n2->begin();
    for (; n2it != n2->end(); ++n2it) {
      if (n1->count(*n2it) <= 0) {
        return false;
      }
    }
    return true;
  }
  bool implies(const Pattern &rhs) const {
    if (this->atom.getType() != rhs.atom.getType()) {
      return false;
    }
    size_t arity = this->atom.getArity();
    if (arity != rhs.atom.getArity()) {
      return false;
    }
    size_t i;
    for (i = this->atom.startPart(); i <= arity; ++i) {
      RIFTerm t1 = this->atom.getPart(i);
      RIFTerm t2 = rhs.atom.getPart(i);
      NMap::const_iterator n1it = this->neqs.find(t1);
      NMap::const_iterator n2it = rhs.neqs.find(t2);
      if (!Pattern::implies(t1, n1it == this->neqs.end() ? NULL : &(n1it->second),
                            t2, n2it ==   rhs.neqs.end() ? NULL : &(n2it->second))) {
        return false;
      }
    }
    return true;
  }
  static bool overlaps(const RIFTerm &t1, const TSet *n1,
                       const RIFTerm &t2, const TSet *n2) {
    return (!t1.isGround() && !t2.isGround())
           || Pattern::implies(t1, n1, t2, n2)
           || Pattern::implies(t2, n2, t1, n1);
  }
  bool overlaps(const Pattern &rhs) const {
    if (this->atom.getType() != rhs.atom.getType()) {
      return false;
    }
    size_t arity = this->atom.getArity();
    if (arity != rhs.atom.getArity()) {
      return false;
    }
    size_t i;
    for (i = this->atom.startPart(); i <= arity; ++i) {
      RIFTerm t1 = this->atom.getPart(i);
      RIFTerm t2 = rhs.atom.getPart(i);
      NMap::const_iterator n1it = this->neqs.find(t1);
      NMap::const_iterator n2it = rhs.neqs.find(t2);
      if (!Pattern::overlaps(t1, n1it == this->neqs.end() ? NULL : &(n1it->second),
                             t2, n2it ==   rhs.neqs.end() ? NULL : &(n2it->second))) {
        return false;
      }
    }
    return true;
  }
  bool operator<(const Pattern &rhs) const {
    if (this->atom.getType() != rhs.atom.getType()) {
      return this->atom.getType() < rhs.atom.getType();
    }
    size_t arity = this->atom.getArity();
    if (arity != rhs.atom.getArity()) {
      return arity < rhs.atom.getArity();
    }
    int c, i;
    for (i = this->atom.startPart(); i <= arity; ++i) {
      RIFTerm t1 = this->atom.getPart(i);
      RIFTerm t2 = rhs.atom.getPart(i);
      if (t1.getType() != t2.getType()) {
        return t1.getType() < t2.getType();
      }
      if (t1.getType() != VARIABLE && t1.getType() != FUNCTION) {
        c = RIFTerm::cmp(t1, t2);
        if (c != 0) {
          return c < 0;
        }
      } else {
        NMap::const_iterator m1it = this->neqs.find(t1);
        NMap::const_iterator m2it = rhs.neqs.find(t2);
        bool empty1 = m1it == this->neqs.end() || m1it->second.empty();
        bool empty2 = m2it == rhs.neqs.end() || m2it->second.empty();
        if (!empty1 && empty2) {
          return false;
        }
        if (empty1 && !empty2) {
          return true;
        }
        if (!empty1 && !empty2) {
          if (m1it->second.size() != m2it->second.size()) {
            return m1it->second.size() < m2it->second.size();
          }
          TSet::const_iterator it1 = m1it->second.begin();
          TSet::const_iterator it2 = m2it->second.begin();
          while (it1 != m1it->second.end()) {
            c = RIFTerm::cmp(*it1, *it2);
            if (c != 0) {
              return c < 0;
            }
            ++it1;
            ++it2;
          }
        }
      }
    }
    return false;
  }
  Pattern &operator=(const Pattern &rhs) {
    NMap nm(rhs.neqs);
    this->atom = rhs.atom;
    this->neqs.swap(nm);
    return *this;
  }
};

ostream &operator<<(ostream &stream, const Pattern &pattern) {
  stream << "And(";
  stream << pattern.atom;
  NMap::const_iterator it = pattern.neqs.begin();
  NMap::const_iterator end = pattern.neqs.end();
  for (; it != end; ++it) {
    stream << " Not(External(\"http://www.w3.org/2007/rif-builtin-predicate#list-contains\"^^<http://www.w3.org/2007/rif#iri>(List(";
    TSet::const_iterator it2 = it->second.begin();
    for (; it2 != it->second.end(); ++it2) {
      if (it2 != it->second.begin()) {
        stream << ' ';
      }
      stream << *it2;
    }
    stream << ") " << it->first << ")))";
  }
  stream << ')';
  return stream;
}

enum Pred {
  REPLICATE = 0,
  PROBLEM = 1,
  ARBITRARY = 2,
  SPLIT = 3,
};

class Label {
public:
  Pattern pattern;
  enum Pred pred;
  Label(const enum Pred p, const RIFAtomic &atom)
      : pattern(Pattern(atom)), pred(p) {
    // do nothing
  }
  Label(const enum Pred p, const RIFAtomic &atom, const NMap &neqs)
      : pattern(Pattern(atom, neqs)), pred(p) {
    // do nothing
  }
  Label(const enum Pred p, const Pattern &patt)
      : pattern(patt), pred(p) {
    // do nothing
  }
  Label(const Label &copy)
      : pattern(copy.pattern), pred(copy.pred) {
    // do nothing
  }
  ~Label() {
    // do nothing
  }
  bool operator<(const Label &rhs) const {
    if (this->pred != rhs.pred) {
      return this->pred < rhs.pred;
    }
    return this->pattern < rhs.pattern;
  }
};

ostream &operator<<(ostream &stream, Label &label) {
  switch (label.pred) {
    case REPLICATE: stream << "REPLICATE"; break;
    case ARBITRARY: stream << "ARBITRARY"; break;
    case SPLIT: stream << "SPLIT"; break;
    case PROBLEM: stream << "PROBLEM"; break;
    default: THROW(TraceableException, "This should never happen.");
  }
  return stream << ' ' << label.pattern;
}

typedef set<RIFRule,
            bool (*)(const RIFRule &, const RIFRule &)>
        RuleSet;
#define RULESET(varname) RuleSet varname(RIFRule::cmplt0)

map<Pattern, size_t> label2num;
map<Pattern, enum Pred> label2pred;
deque<Pattern> num2label;
size_t numclauses = 0;
stringstream sout (stringstream::in | stringstream::out);

bool is_fixed_pattern(const Pattern &patt, enum Pred &pred) {
  map<Pattern, enum Pred>::iterator it = label2pred.find(patt);
  if (it == label2pred.end()) {
    return false;
  }
  pred = it->second;
  return true;
}

int possibility(const bool sign, const enum Pred pred, const Pattern &patt) {
  enum Pred p;
  if (is_fixed_pattern(patt, p)) {
    if (pred == PROBLEM) {
      return p == SPLIT ? 0 : (sign ? -1 : 1);
    }
    return ((sign && pred == p) || (!sign && pred != p)) ? 1 : -1;
  }
  return 0;
}

bool print_literal(const bool sign, const enum Pred pred, const Pattern &patt) {
  enum Pred p;
  if (possibility(sign, pred, patt) < 0) {
    return false;
  }
  map<Pattern, size_t>::const_iterator it = label2num.find(patt);
  size_t id;
  if (it == label2num.end()) {
    id = num2label.size();
    label2num[patt] = id;
    num2label.push_back(patt);
  } else {
    id = it->second;
  }
#if !DECIDE
  id <<= 2;
#endif
  id += 1 + (int)pred;
  if (!sign) {
    sout << '-';
  }
  sout << id;
  return true;
}

void termc() {
  sout << " 0" << endl;
  ++numclauses;
}

#if DECIDE
void fix_pattern(const enum Pred pred, const Pattern &patt) {
  if (pred != REPLICATE) {
    cerr << "[ERROR] When in decision mode, only the pred REPLICATE can be used, but found " << pred << endl;
    THROW(TraceableException, "In decision mode, on the pred REPLICATE can be used.");
  }
  enum Pred p;
  if (is_fixed_pattern(patt, p)) {
    return;
  }
  print_literal(true, pred, patt);
  termc();
  label2pred[patt] = pred;
}
#else
void fix_pattern(const enum Pred pred, const Pattern &patt) {
  if (pred == PROBLEM) {
    cerr << "[ERROR] Not allowed to fix value to PROBLEM, as you just tried with " << patt << endl;
    THROW(TraceableException, "Not allowed to fix value to PROBLEM.");
  }
  enum Pred p;
  if (is_fixed_pattern(patt, p)) {
    if (p != pred) {
      cerr << "[ERROR] Tried to set value of " << patt << " to " << pred << " when already set to " << pred << endl;
      THROW(TraceableException, "Conflicted values set.");
    }
    return;
  }
  print_literal(true, pred, patt);
  termc();
  switch (pred) {
    case REPLICATE:
      print_literal(false, PROBLEM, patt); termc();
      print_literal(false, ARBITRARY, patt); termc();
      print_literal(false, SPLIT, patt); termc();
      break;
    case ARBITRARY:
      print_literal(false, REPLICATE, patt); termc();
      print_literal(false, PROBLEM, patt); termc();
      print_literal(false, SPLIT, patt); termc();
      break;
    case SPLIT:
      print_literal(false, REPLICATE, patt); termc();
      print_literal(false, ARBITRARY, patt); termc();
      break;
  }
  label2pred[patt] = pred;
}
#endif

void print_clause(const bool sign1, const enum Pred pred1, const Pattern &patt1) {
  if (possibility(sign1, pred1, patt1) > 0) {
    return; // clause is redundant; no need to print it
  }
  enum Pred p;
  if (!print_literal(sign1, pred1, patt1)) {
    cerr << "[ERROR] Contradiction in generating SAT formula.  " << patt1 << endl;
    THROW(TraceableException, "Contradiction in generating SAT formula.");
  }
  termc();
}

void print_clause(const bool sign1, const enum Pred pred1, const Pattern &patt1,
                  const bool sign2, const enum Pred pred2, const Pattern &patt2) {
  if (possibility(sign1, pred1, patt1) > 0 &&
      possibility(sign2, pred2, patt2) > 0 ) {
    return; // clause is redundant; no need to print it
  }
  bool printed1 = print_literal(sign1, pred1, patt1);
  if (printed1) {
    sout << ' ';
  }
  bool printed2 = print_literal(sign2, pred2, patt2);
  if (!printed1 && !printed2) {
    cerr << "[ERROR] Contradiction in generating SAT formula.  " << patt1 << " *OR* " << patt2 << endl;
    THROW(TraceableException, "Contradiction in generating SAT formula.");
  }
  termc();
}

void print_clause(const bool sign1, const enum Pred pred1, const Pattern &patt1,
                  const bool sign2, const enum Pred pred2, const Pattern &patt2,
                  const bool sign3, const enum Pred pred3, const Pattern &patt3) {
  if (possibility(sign1, pred1, patt1) > 0 &&
      possibility(sign2, pred2, patt2) > 0 &&
      possibility(sign3, pred3, patt3) > 0 ) {
    return; // clause is redundant; no need to print it
  }
  bool printed1 = print_literal(sign1, pred1, patt1);
  if (printed1) {
    sout << ' ';
  }
  bool printed2 = print_literal(sign2, pred2, patt2);
  if (printed2) {
    sout << ' ';
  }
  bool printed3 = print_literal(sign3, pred3, patt3);
  if (!printed1 && !printed2 && !printed3) {
    cerr << "[ERROR] Contradiction in generating SAT formula.  " << patt1 << " *OR* " << patt2 << endl;
    THROW(TraceableException, "Contradiction in generating SAT formula.");
  }
  termc();
}

void condstr2patt(string str, set<Pattern> &patterns) {
  cerr << "[INFO] REPLICATE: ";
  DPtr<uint8_t> *p = cstr2dptr(str.c_str());
  printstr(cerr, p);
  cerr << endl;
  RIFCondition cond = RIFCondition::parse(p);
  if (cond.getType() != CONJUNCTION) {
    cerr << "[ERROR] Replicate pragma should have conjunction formula instead of " << cond << "." << endl;
    THROW(TraceableException, "Replicate pragma should have conjunction formula.");
  }
  DPtr<RIFCondition> *subc = cond.getSubformulas();
  if (subc->size() < 1) {
    cerr << "[ERROR] Replicate pragma should have non-empty conjunction formula." << endl;
    THROW(TraceableException, "Replicate pragma should have non-empty conjuntion formula.");
  }
  NMAP(neqs);
  const RIFCondition *sit = subc->dptr();
  const RIFCondition *end = sit + subc->size();
  if (sit->getType() != ATOMIC) {
    cerr << "[ERROR] Invalid formula in replicate pragma: " << *sit << endl;
    THROW(TraceableException, "Invalid formula in replicate pragma.");
  }
  RIFAtomic atom = sit->getAtomic();
  if (isFixed(atom)) {
    cerr << "[ERROR] First atomic formula in replicate pragma should not be \"fixed\": " << atom << endl;
    THROW(TraceableException, "First atomic formula in replicate pragma should not be \"fixed\".");
  }
  VarSet vars(RIFVar::cmplt0);
  atom.getVars(vars);
  for (++sit; sit != end; ++sit) {
    if (sit->getType() != NEGATION) {
      cerr << "[ERROR] Subformulas in replicate pragma should be negated atomic, but found: " << *sit << endl;
      THROW(TraceableException, "Subformulas in replicate pragma should be negated atomic.");
    }
    RIFCondition negc = sit->getSubformula();
    if (negc.getType() != ATOMIC) {
      cerr << "[ERROR] Subformulas in replicate pragma should be negated atomic, but found: " << negc << endl;
      THROW(TraceableException, "Subformulas in replicate pragma should be negated atomic.");
    }
    RIFAtomic restriction = negc.getAtomic();
    if (restriction.getType() != EXTERNAL) {
      cerr << "[ERROR] Non-first atomic formula in replicate pragma should be builtin: " << restriction << endl;
      THROW(TraceableException, "Non-first atomic formula in replicate pragma should be builtin.");
    }
    if (!isPredListContains(restriction.getPred())) {
      cerr << "[ERROR] Expected pred:list-contains but found: " << restriction << endl;
      THROW(TraceableException, "Expected pred:list-countains but found something else in restriction pragma.");
    }
    DPtr<RIFTerm> *args = restriction.getArgs();
    if (args->size() != 2) {
      cerr << "[ERROR] pred:list-contains should have exactly two arguments, but found: " << restriction << endl;
      THROW(TraceableException, "pred:list-contains should have exactly two arguments.");
    }
    RIFTerm *arg = args->dptr();
    if (arg->getType() != LIST) {
      cerr << "[ERROR] First arg of pred:list-contains should be a list, but found: " << *arg << endl;
      THROW(TraceableException, "First arg or pred:list-contains should be a list.");
    }
    DPtr<RIFTerm> *items = arg->getItems();
    ++arg;
    if (arg->getType() != VARIABLE) {
      cerr << "[ERROR] Second arg of pred:list-contains should be a variable in this case, but found: " << *arg << endl;
      THROW(TraceableException, "Second arg of pred:list-contains should be a variable in this case.");
    }
    if (vars.count(arg->getVar()) <= 0) {
      cerr << "[ERROR] Variable in pred:list-contains should be a variable in the first formula, but found: " << atom << " and " << *arg << endl;
      THROW(TraceableException, "Variable in pred:list-contains should be a variable in the first formula.");
    }
    if (neqs.count(*arg) <= 0) {
      neqs.insert(pair<RIFTerm, TSet>(*arg, TSet(RIFTerm::cmplt0)));
    }
    RIFTerm *item = items->dptr();
    RIFTerm *enditems = item + items->size();
    for (; item != enditems; ++item) {
      neqs[*arg].insert(*item);
    }
    args->drop();
    items->drop();
  }
  Pattern patt(atom, neqs);
  patterns.insert(patt);
  subc->drop();
}

#if DECIDE
void handle_pragma(string str, set<Pattern> &patterns) {
  if (str.size() >= 10 && str.substr(0, 10) == string("REPLICATE ")) {
    set<Pattern> patts;
    condstr2patt(str.substr(10), patts);
    set<Pattern>::const_iterator it = patts.begin();
    for (; it != patts.end(); ++it) {
      fix_pattern(REPLICATE, *it);
    }
    patterns.insert(patts.begin(), patts.end());
  } else if (str.size() >= 10 && str.substr(0, 10) == string("ARBITRARY ")) {
    set<Pattern> patts;
    condstr2patt(str.substr(10), patts);
    set<Pattern>::const_iterator it = patts.begin();
    for (; it != patts.end(); ++it) {
      print_clause(false, REPLICATE, *it);
    }
    patterns.insert(patts.begin(), patts.end());
  } else if (str.size() >= 6 && str.substr(0, 6) == string("SPLIT ")) {
    set<Pattern> patts;
    condstr2patt(str.substr(6), patts);
    set<Pattern>::const_iterator it = patts.begin();
    for (; it != patts.end(); ++it) {
      print_clause(false, REPLICATE, *it);
    }
    patterns.insert(patts.begin(), patts.end());
  } else {
    cerr << "[WARNING] Ignoring unrecognized pragma: " << str << endl;
  }
}
#else
void handle_pragma(string str, set<Pattern> &patterns) {
  if (str.size() >= 10 && str.substr(0, 10) == string("REPLICATE ")) {
    set<Pattern> patts;
    condstr2patt(str.substr(10), patts);
    set<Pattern>::const_iterator it = patts.begin();
    for (; it != patts.end(); ++it) {
      fix_pattern(REPLICATE, *it);
    }
    patterns.insert(patts.begin(), patts.end());
  } else if (str.size() >= 10 && str.substr(0, 10) == string("ARBITRARY ")) {
    set<Pattern> patts;
    condstr2patt(str.substr(10), patts);
    set<Pattern>::const_iterator it = patts.begin();
    for (; it != patts.end(); ++it) {
      fix_pattern(ARBITRARY, *it);
    }
    patterns.insert(patts.begin(), patts.end());
  } else if (str.size() >= 6 && str.substr(0, 6) == string("SPLIT ")) {
    set<Pattern> patts;
    condstr2patt(str.substr(6), patts);
    set<Pattern>::const_iterator it = patts.begin();
    for (; it != patts.end(); ++it) {
      fix_pattern(SPLIT, *it);
    }
    patterns.insert(patts.begin(), patts.end());
  } else {
    cerr << "[WARNING] Ignoring unrecognized pragma: " << str << endl;
  }
}
#endif

void read_rules(vector<RIFRule> &rules, set<Pattern> &patterns) {
  string line;
  while (!cin.eof()) {
    getline(cin, line);
    if (line.size() >= 8 && line.substr(0, 8) == string("#PRAGMA ")) {
      handle_pragma(line.substr(8), patterns);
      continue;
    } else if (line.size() >= 1 && line[0] == '#') {
      // ignore comment
      continue;
    } else if (line.size() <= 1) {
      break;
    }
    cerr << "[INFO] PARSING: ";
    DPtr<uint8_t> *p = cstr2dptr(line.c_str());
    printstr(cerr, p);
    cerr << endl;
    RIFRule rule = RIFRule::parse(p);
    rules.push_back(rule);
    p->drop();
  }
}

void force_nontrivial_solution(set<Pattern> &patterns) {
  DPtr<uint8_t> *p = cstr2dptr("?s[?p->?o]");
  RIFAtomic atom = RIFAtomic::parse(p);
  p->drop();
  Pattern patt(atom);
  print_clause(false, REPLICATE, patt);
  patterns.insert(patt);
}

void generate_replication_clauses(const multiset<Pattern> &patts) {
  set<Pattern>::const_iterator it = patts.begin();
  for (; it != patts.end(); ++it) {
    print_clause(true, REPLICATE, *it);
  }
}

void generate_condition_based_clauses(const multiset<Pattern> &patts) {
  set<Pattern>::const_iterator it = patts.begin();
  for (; it != patts.end(); ++it) {
    set<Pattern>::const_iterator it2 = patts.begin();
    for (; it2 != patts.end(); ++it2) {
      if (it != it2) {
        print_clause(true, REPLICATE, *it,
                        true, REPLICATE, *it2);
      }
    }
  }
}

#if DECIDE
void generate_action_based_clauses(const multiset<Pattern> &bpatts, const multiset<Pattern> &hpatts) {
  set<Pattern>::const_iterator hit = hpatts.begin();
  for (; hit != hpatts.end(); ++hit) {
    set<Pattern>::const_iterator bit = bpatts.begin();
    for (; bit != bpatts.end(); ++bit) {
      print_clause(false, REPLICATE, *hit,
                      true, REPLICATE, *bit);
    }
  }
}
#else
void generate_action_based_clauses(const multiset<Pattern> &bpatts, const multiset<Pattern> &hpatts) {
  set<Pattern>::const_iterator hit = hpatts.begin();
  for (; hit != hpatts.end(); ++hit) {
    print_clause(false, SPLIT, *hit,
                    true, PROBLEM, *hit);
    set<Pattern>::const_iterator bit = bpatts.begin();
    for (; bit != bpatts.end(); ++bit) {
      print_clause(false, REPLICATE, *hit,
                      true, REPLICATE, *bit);
      print_clause(true, ARBITRARY, *hit,
                      false, ARBITRARY, *bit);
    }
  }
}
#endif

void generate_rule_based_clauses(const multiset<Pattern> &cpatts, const multiset<Pattern> &apatts,
                                 const multiset<Pattern> &npatts, const multiset<Pattern> &rpatts) {
  generate_replication_clauses(npatts);
  generate_replication_clauses(rpatts);
  generate_condition_based_clauses(cpatts);
  generate_condition_based_clauses(npatts);
  generate_action_based_clauses(cpatts, apatts);
  generate_action_based_clauses(npatts, apatts);
  generate_action_based_clauses(cpatts, rpatts);
  generate_action_based_clauses(npatts, rpatts);
}

#if DECIDE
void generate_pattern_based_clauses(const set<Pattern> &patterns) {
  set<Pattern>::const_iterator it1 = patterns.begin();
  for (; it1 != patterns.end(); ++it1) {
    set<Pattern>::const_iterator it2 = patterns.begin();
    for (; it2 != patterns.end(); ++it2) {
      if (it1 != it2) {
        if (it2->implies(*it1)) {
          cerr << "[INFO] IMPLY? " << *it2 << ' ' << *it1 << endl;
          print_clause(false, REPLICATE, *it1,
                          true, REPLICATE, *it2);
        }
      }
    }
  }
}
#else
void generate_pattern_based_clauses(const set<Pattern> &patterns) {
  set<Pattern>::const_iterator it1 = patterns.begin();
  for (; it1 != patterns.end(); ++it1) {
#if NOPROBLEMS
    print_clause(false, PROBLEM, *it1);
#endif
    print_clause(true, SPLIT, *it1,
                    false, PROBLEM, *it1);
    print_clause(true, ARBITRARY, *it1,
                    true, REPLICATE, *it1,
                    true, SPLIT, *it1);
    print_clause(false, SPLIT, *it1,
                    false, REPLICATE, *it1);
    print_clause(false, SPLIT, *it1,
                    false, ARBITRARY, *it1);
    print_clause(false, REPLICATE, *it1,
                    false, ARBITRARY, *it1);
    if (it1->atom.isGround()) {
      print_clause(false, SPLIT, *it1);
    }
    set<Pattern>::const_iterator it2 = patterns.begin();
    for (; it2 != patterns.end(); ++it2) {
      if (it1 != it2) {
        if (it2->implies(*it1)) {
          cerr << "[INFO] IMPLY? " << *it2 << ' ' << *it1 << endl;
          print_clause(false, REPLICATE, *it1,
                          true, REPLICATE, *it2);
          print_clause(false, ARBITRARY, *it1,
                          true, ARBITRARY, *it2);
        }
        if (it2->overlaps(*it1)) {
          cerr << "[INFO] OVERLAP? " << *it2 << ' ' << *it1 << endl;
          print_clause(false, REPLICATE, *it1,
                          false, ARBITRARY, *it2);
          print_clause(false, ARBITRARY, *it1,
                          false, REPLICATE, *it2);
        }
      }
    }
  }
}
#endif

void extract_restrictions(const RIFCondition &condition, NMap &restrictions) {
  try {
    switch (condition.getType()) {
    case ATOMIC: {
      RIFAtomic a = condition.getAtomic();
      if (a.getType() == EXTERNAL) {
        cerr << "[WARNING] Admitting builtin, but acting as though it is not present.  Thus, if SAT formula is satisfied, distribution scheme works.  If SAT formula is not satisfied, then distribution scheme may or may not work." << endl;
      }
      break;
    }
    case CONJUNCTION: {
      DPtr<RIFCondition> *subs = condition.getSubformulas();
      RIFCondition *cond = subs->dptr();
      RIFCondition *end = cond + subs->size();
      for (; cond != end; ++cond) {
        extract_restrictions(*cond, restrictions);
      }
      subs->drop();
      break;
    }
    case NEGATION: {
      RIFCondition cond = condition.getSubformula();
      if (cond.getType() != ATOMIC) {
        break;
      }
      RIFAtomic atom = cond.getAtomic();
      if (atom.getType() != EXTERNAL) {
        break;
      }
      RIFConst pred = atom.getPred();
      if (!isPredListContains(pred)) {
        break;
      }
      DPtr<RIFTerm> *args = atom.getArgs();
      if (args->size() != 2) {
        THROW(TraceableException, "Invalid use of pred:list-contains.");
      }
      RIFTerm *arg = args->dptr();
      if (arg->getType() != LIST) {
        THROW(TraceableException, "Invalid use of pred:list-contains.");
      }
      DPtr<RIFTerm> *items = arg->getItems();
      ++arg;
      if (arg->isGround()) {
        cerr << "[WARNING] Check to make sure this is false: " << atom << endl;
        // THROW(TraceableException, "Invalid use of pred:list-contains.");
      }
      //RIFVar var = arg->getVar();
      RIFTerm var = *arg;  // var could also be a non-ground function
      args->drop();
      RIFTerm *item = items->dptr();
      RIFTerm *enditems = item + items->size();
      for (; item != enditems; ++item) {
        NMap::iterator mit = restrictions.find(var);
        if (mit == restrictions.end()) {
          // mit = restrictions.insert(pair<RIFVar, TSet>(var, TSet(RIFTerm::cmplt0))).first;
          mit = restrictions.insert(pair<RIFTerm, TSet>(var, TSet(RIFTerm::cmplt0))).first;
        }
        mit->second.insert(*item);
      }
      items->drop();
      break;
    }
    default: THROW(TraceableException, "Unsupported case.");
    }
  } JUST_RETHROW(TraceableException, "(rethrow)")
}

void extract_patterns(const RIFAtomic &atom, const NMap &restrictions, multiset<Pattern> &patts) {
  try {
    if (isFixed(atom)) {
      return;
    }
    NMap neqs (RIFTerm::cmplt0);
    VarSet vars (RIFVar::cmplt0);
    atom.getVars(vars);
    VarSet::const_iterator it = vars.begin();
    for (; it != vars.end(); ++it) {
      NMap::const_iterator rit = restrictions.find(*it);
      if (rit != restrictions.end()) {
        neqs.insert(*rit);
      }
    }
    Pattern patt(atom, neqs);
    cerr << "[INFO] Inserting pattern: " << patt << endl;
    patts.insert(patt);
  } JUST_RETHROW(TraceableException, "(rethrow)")
}

void extract_patterns(const RIFCondition &condition, const NMap &restrictions,
                      multiset<Pattern> &patts, multiset<Pattern> &npatts) {
  try {
    switch (condition.getType()) {
    case ATOMIC: {
      extract_patterns(condition.getAtomic(), restrictions, patts);
      break;
    }
    case CONJUNCTION: {
      DPtr<RIFCondition> *subs = condition.getSubformulas();
      RIFCondition *cond = subs->dptr();
      RIFCondition *end = cond + subs->size();
      for (; cond != end; ++cond) {
        extract_patterns(*cond, restrictions, patts, npatts);
      }
      subs->drop();
      break;
    }
    case NEGATION: {
      extract_patterns(condition.getSubformula(), restrictions, npatts, npatts);
      break;
    }
    default: {
      cerr << "[ERROR] Unsupported condition formula structure: " << condition << endl;
      THROW(TraceableException, "Unsupported condition formula structure.");
    }
    }
  } JUST_RETHROW(TraceableException, "(rethrow)")
}

void extract_patterns(const RIFActionBlock &actionblock, const NMap &restrictions,
                      multiset<Pattern> &patts, multiset<Pattern> &rpatts) {
  try {
    DPtr<RIFAction> *actions = actionblock.getActions();
    RIFAction *action = actions->dptr();
    RIFAction *end = action + actions->size();
    for (; action != end; ++action) {
      if (action->getType() == ASSERT_FACT) {
        extract_patterns(action->getTargetAtomic(), restrictions, patts);
      } else if (action->getType() == RETRACT_FACT) {
        extract_patterns(action->getTargetAtomic(), restrictions, rpatts);
      } else {
        cerr << "[ERROR] Handling only fact assertion and retraction." << endl;
        THROW(TraceableException, "Handling only fact assertion and retraction.");
      }
    }
    actions->drop();
  } JUST_RETHROW(TraceableException, "(rethrow)")
}

void extract_patterns(const RIFRule &rule, multiset<Pattern> &cpatts, multiset<Pattern> &apatts,
                                           multiset<Pattern> &npatts, multiset<Pattern> &rpatts) {
  try {
    NMap restrictions (RIFTerm::cmplt0);
    RIFCondition condition = rule.getCondition();
    RIFActionBlock actionblock = rule.getActionBlock();
    extract_restrictions(condition, restrictions);
    extract_patterns(condition, restrictions, cpatts, npatts);
    extract_patterns(actionblock, restrictions, apatts, rpatts);
  } JUST_RETHROW(TraceableException, "(rethrow)")
}

#if DECIDE
void print_cnf() {
  size_t i;
  for (i = 0; i < num2label.size(); ++i) {
    cout << "c " << i+1 << " REPLICATE " << num2label[i] << endl;
  }
  cout << "p cnf " << num2label.size() << ' ' << numclauses << endl;
  cout << sout.str();
}
#else
void print_cnf() {
  size_t i;
  for (i = 0; i < num2label.size(); ++i) {
    size_t j = i << 2;
    cout << "c " << j+1 << " REPLICATE " << num2label[i] << endl
         << "c " << j+2 << " PROBLEM "   << num2label[i] << endl
         << "c " << j+3 << " ARBITRARY " << num2label[i] << endl
         << "c " << j+4 << " SPLIT "     << num2label[i] << endl;
  }
  cout << "p cnf " << 4*num2label.size() << ' ' << numclauses << endl;
  cout << sout.str();
}
#endif

int prd2cnf(int argc, char **argv) {
  vector<RIFRule> rules;
  set<Pattern> patterns;

  read_rules(rules, patterns);
#if NONTRIVIAL
  force_nontrivial_solution(patterns);
#endif
  vector<RIFRule>::const_iterator it = rules.begin();
  for (; it != rules.end(); ++it) {
    multiset<Pattern> cpatts, apatts, npatts, rpatts;
    extract_patterns(*it, cpatts, apatts, npatts, rpatts);
    patterns.insert(cpatts.begin(), cpatts.end());
    patterns.insert(apatts.begin(), apatts.end());
    patterns.insert(npatts.begin(), npatts.end());
    patterns.insert(rpatts.begin(), rpatts.end());
    generate_rule_based_clauses(cpatts, apatts, npatts, rpatts);
  }
  generate_pattern_based_clauses(patterns);

  print_cnf();
  return 0;
}
