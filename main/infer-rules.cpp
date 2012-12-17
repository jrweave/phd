#include <algorithm>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "main/encode.h"

#ifdef DEBUG
#undef DEBUG
#endif
//#define DEBUG(msg, val) cerr << "[DEBUG] " << __FILE__ << ':' << __LINE__ << ": " << (msg) << (val) << endl
#define DEBUG(msg, val)

using namespace std;

// The must match RIFTermType in RIFTerm.h.
#define VARIABLE 0
#define CONSTANT 1
#define LIST 2
#define FUNCTION 3

// These must match RIFAtomicType in RIFAtomic.h.
#define ATOM 0
#define EXTERNAL 1
#define EQUALITY 2
#define MEMBERSHIP 3
#define SUBCLASS 4
#define FRAME 5

// These must match RIFCondType in RIFCondition.h.
#define ATOMIC 0
#define CONJUNCTION 1
#define DISJUNCTION 2
#define NEGATION 3
#define EXISTENTIAL 4

// These must match RIFActType in RIFAction.h.
#define ASSERT_FACT 0
#define RETRACT_FACT 1
#define RETRACT_SLOTS 2
#define RETRACT_OBJECT 3
#define EXECUTE 4
#define MODIFY 5

struct Function;
struct Term;
struct Frame;
struct Atom;
struct Builtin;
struct Atomic;
struct Execution;
struct Action;
struct ActionVariableBinding;
struct ActionBlock;
struct Condition;
struct Rule;

ostream &operator<<(ostream &stream, Function&);
ostream &operator<<(ostream &stream, Term&);
ostream &operator<<(ostream &stream, Frame&);
ostream &operator<<(ostream &stream, Atom&);
ostream &operator<<(ostream &stream, Builtin&);
ostream &operator<<(ostream &stream, Atomic&);
ostream &operator<<(ostream &stream, Execution&);
ostream &operator<<(ostream &stream, Action&);
ostream &operator<<(ostream &stream, ActionVariableBinding&);
ostream &operator<<(ostream &stream, ActionBlock&);
ostream &operator<<(ostream &stream, Condition&);
ostream &operator<<(ostream &stream, Rule&);

template<typename T>
ostream &operator<<(ostream &stream, pair<T, T> &p) {
  return stream << "{ first = " << p.first << " ; second = " << p.second << " }";
}

template<typename T>
struct range_of {
  T *begin, *end;
};

template<typename T>
ostream &operator<<(ostream &stream, range_of<T> &range) {
  T *it;
  stream << "[ ";
  for (it = range.begin; it != range.end; ++it) {
    if (it != range.begin) {
      stream << " ; ";
    }
    stream << *it;
  }
  stream << " ]";
  return stream;
}

template<typename myint_t>
string hexify(const myint_t &n) {
  stringstream ss (stringstream::in | stringstream::out);
  ss << setfill('0');
  ss << setw(sizeof(myint_t) << 1) << hex << (int64_t)n << flush;
  return ss.str();
}

struct Function {
  funcint_t name;
  range_of<Term> arguments;
};

ostream &operator<<(ostream &stream, Function &function) {
  stream << "{ name = " << hexify(function.name) << " ; ";
  stream << "arguments = " << function.arguments << " }";
  return stream;
}

struct Term {
  termtypeint_t type;
  union {
    Function function;
    range_of<Term> termlist;
    varint_t variable;
    constint_t constant;
  } get;
};

ostream &operator<<(ostream &stream, Term &term) {
  stream << "{ type = " << hexify(term.type) << " ; ";
  switch (term.type) {
    case VARIABLE:
      stream << "variable = " << hexify(term.get.variable);
      break;
    case CONSTANT:
      stream << "constant = " << hexify(term.get.constant);
      break;
    case LIST:
      stream << "list = " << term.get.termlist;
      break;
    case FUNCTION:
      stream << "function = " << term.get.function;
      break;
    default:
      stream<< "(*ERROR* Unhandled case at line " << __LINE__ << ")";
      break;
  }
  stream << " }";
  return stream;
}

struct Frame {
  Term object;
  range_of<pair<Term, Term> > slots;
};

ostream &operator<<(ostream &stream, Frame &frame) {
  stream << "{ object = " << frame.object << " ; ";
  stream << "slots = " << frame.slots << " }";
  return stream;
}

struct Atom {
  constint_t predicate;
  range_of<Term> arguments;
};

ostream &operator<<(ostream &stream, Atom &atom) {
  stream << "{ predicate = " << hexify(atom.predicate) << " ; ";
  stream << "arguments = " << atom.arguments << " }";
  return stream;
}

struct Builtin {
  builtint_t predicate;
  range_of<Term> arguments;
};

ostream &operator<<(ostream &stream, Builtin &builtin) {
  stream << "{ predicate = " << hexify(builtin.predicate) << " ; ";
  stream << "arguments = " << builtin.arguments << " }";
  return stream;
}

struct Atomic {
  atomtypeint_t type;
  union {
    Atom atom;
    Builtin builtin;
    Term sides[2];
    Frame frame;
  } get;
};

ostream &operator<<(ostream &stream, Atomic &atom) {
  stream << "{ type = " << hexify(atom.type) << " ; ";
  switch (atom.type) {
    case ATOM:
      stream << "atom = " << atom.get.atom;
      break;
    case EXTERNAL:
      stream << "builtin = " << atom.get.builtin;
      break;
    case FRAME:
      stream << "frame = " << atom.get.frame;
      break;
    case EQUALITY:
    case MEMBERSHIP:
    case SUBCLASS:
      stream << "sides = [ " << atom.get.sides[0] << " ; " << atom.get.sides[1] << " ]";
      break;
    default:
      stream << "(*ERROR* Unhandled case at line " << __LINE__ << ")";
      break;
  }
  stream << " }";
  return stream;
}

struct Execution {
  execint_t predicate;
  range_of<Term> arguments;
};

ostream &operator<<(ostream &stream, Execution &execution) {
  stream << "{ predicate = " << hexify(execution.predicate) << " ; ";
  stream << "arguments = " << execution.arguments << " }";
  return stream;
}

struct Action {
  acttypeint_t type;
  union {
    Atomic atom;
    Term terms[2];
    Term term;
    Execution execution;
  } get;
};

ostream &operator<<(ostream &stream, Action &action) {
  stream << "{ type = " << hexify(action.type) << " ; ";
  switch (action.type) {
    case ASSERT_FACT:
    case RETRACT_FACT:
    case MODIFY:
      stream << "atom = " << action.get.atom;
      break;
    case RETRACT_SLOTS:
      stream << "terms = [ " << action.get.terms[0] << " ; " << action.get.terms[1] << " ]";
      break;
    case RETRACT_OBJECT:
      stream << "term = " << action.get.term;
      break;
    case EXECUTE:
      stream << "execution = " << action.get.execution;
      break;
    default:
      stream << "(*ERROR* Unhandled case at line " << __LINE__ << ")";
      break;
  }
  stream << " }";
  return stream;
}

struct ActionVariableBinding {
  varint_t variable;
  bool newness;
  Atomic atom;
};

ostream &operator<<(ostream &stream, ActionVariableBinding &binding) {
  stream << "{ variable = " << hexify(binding.variable);
  if (!binding.newness) {
    stream << " ; atom = " << binding.atom;
  }
  stream << " }";
  return stream;
}

struct ActionBlock {
  range_of<ActionVariableBinding> action_variables;
  range_of<Action> actions;
};

ostream &operator<<(ostream &stream, ActionBlock &action_block) {
  stream << "{ action_variables = " << action_block.action_variables << " ; ";
  stream << "actions = " << action_block.actions << " }";
  return stream;
}

struct Condition {
  condtypeint_t type;
  union {
    Atomic atom;
    range_of<Condition> subformulas;
  } get;
};

ostream &operator<<(ostream &stream, Condition &condition) {
  stream << "{ type = " << hexify(condition.type) << " ; ";
  switch (condition.type) {
    case ATOMIC: {
      stream << "atom = " << condition.get.atom;
      break;
    }
    case CONJUNCTION:
    case DISJUNCTION: {
      stream << "subformulas = " << condition.get.subformulas;
      break;
    }
    case NEGATION:
    case EXISTENTIAL: {
      stream << "subformula = " << *condition.get.subformulas.begin;
      break;
    }
    default: {
      stream << "(*ERROR* Unhandled case at line " << __LINE__ << ")";
      break;
    }
  }
  stream << " }";
  return stream;
}

struct Rule {
  Condition condition;
  ActionBlock action_block;
};

ostream &operator<<(ostream &stream, Rule &rule) {
  stream << "{ condition = " << rule.condition << " ; ";
  stream << "action_block = " << rule.action_block << " }";
  return stream;
}

template<typename myint_t>
const uint8_t *get_big_endian_int(const uint8_t *bytes, const uint8_t *end, myint_t &n) {
  n = 0;
  const uint8_t *bound = bytes + sizeof(myint_t);
  for (; bytes != bound && bytes != end; ++bytes) {
    n = (n << 8) | ((myint_t) *bytes);
  }
  if (bytes != bound) {
    cerr << "[ERROR] Integer shorter than expected." << endl;
    return NULL;
  }
  DEBUG("n = ", hexify(n));
  return bytes;
}

const uint8_t *build_term(const uint8_t *begin, const uint8_t *end, Term &term) {
  const uint8_t *offset = get_big_endian_int<termtypeint_t>(begin, end, term.type);
  switch (term.type) {
    case VARIABLE: {
      offset = get_big_endian_int(offset, end, term.get.variable);
      break;
    }
    case CONSTANT: {
      offset = get_big_endian_int(offset, end, term.get.constant);
      break;
    }
    case LIST: {
      vector<Term> termlist;
      sizeint_t size;
      offset = get_big_endian_int(offset, end, size);
      const uint8_t *bound = offset + size;
      while (offset != NULL && offset != bound) {
        Term term;
        offset = build_term(offset, bound, term);
        termlist.push_back(term);
      }
      if (offset != bound) {
        cerr << "[ERROR] List term is different length than expected." << endl;
        return NULL;
      }
      term.get.termlist.begin = new Term[termlist.size()];
      term.get.termlist.end = term.get.termlist.begin + termlist.size();
      copy(termlist.begin(), termlist.end(), term.get.termlist.begin);
      break;
    }
    case FUNCTION: {
      vector<Term> arguments;
      sizeint_t size;
      offset = get_big_endian_int(offset, end, size);
      const uint8_t *bound = offset + size;
      offset = get_big_endian_int(offset, bound, term.get.function.name);
      while (offset != NULL && offset != bound) {
        Term term;
        offset = build_term(offset, bound, term);
        arguments.push_back(term);
      }
      if (offset != bound) {
        cerr << "[ERROR] Function term is different length than expected." << endl;
        return NULL;
      }
      term.get.function.arguments.begin = new Term[arguments.size()];
      term.get.function.arguments.end = term.get.function.arguments.begin + arguments.size();
      copy(arguments.begin(), arguments.end(), term.get.function.arguments.begin);
    }
  }
  DEBUG("term = ", term);
  return offset;
}

const uint8_t *build_frame(const uint8_t *begin, const uint8_t *end, Frame &frame) {
  if (begin == NULL) {
    return NULL;
  }
  vector<pair<Term, Term> > slots;
  const uint8_t *offset = build_term(begin, end, frame.object);
  while (offset != NULL && offset != end) {
    pair<Term, Term> slot;
    offset = build_term(offset, end, slot.first);
    offset = build_term(offset, end, slot.second);
    slots.push_back(slot);
  }
  frame.slots.begin = new pair<Term, Term>[slots.size()];
  frame.slots.end = frame.slots.begin + slots.size();
  copy(slots.begin(), slots.end(), frame.slots.begin);
  DEBUG("frame = ", frame);
  return offset;
}

const uint8_t *build_builtin(const uint8_t *begin, const uint8_t *end, Builtin &builtin) {
  if (begin == NULL) {
    return NULL;
  }
  vector<Term> arguments;
  const uint8_t *offset = get_big_endian_int(begin, end, builtin.predicate);
  while (offset != NULL && offset != end) {
    Term term;
    offset = build_term(offset, end, term);
    arguments.push_back(term);
  }
  builtin.arguments.begin = new Term[arguments.size()];
  builtin.arguments.end = builtin.arguments.begin + arguments.size();
  copy(arguments.begin(), arguments.end(), builtin.arguments.begin);
  DEBUG("builtin = ", builtin);
  return offset;
}

const uint8_t *build_atom(const uint8_t *begin, const uint8_t *end, Atom &atom) {
  if (begin == NULL) {
    return NULL;
  }
  vector<Term> arguments;
  const uint8_t *offset = get_big_endian_int(begin, end, atom.predicate);
  while (offset != NULL && offset != end) {
    Term term;
    offset = build_term(offset, end, term);
    arguments.push_back(term);
  }
  atom.arguments.begin = new Term[arguments.size()];
  atom.arguments.end = atom.arguments.begin + arguments.size();
  copy(arguments.begin(), arguments.end(), atom.arguments.begin);
  DEBUG("atom = ", atom);
  return offset;
}

const uint8_t *build_atomic(const uint8_t *begin, const uint8_t *end, Atomic &atom) {
  if (begin == NULL) {
    return NULL;
  }
  sizeint_t size;
  const uint8_t *offset = get_big_endian_int(begin, end, size);
  const uint8_t *bound = offset + size;
  offset = get_big_endian_int(offset, bound, atom.type);
  switch (atom.type) {
    case ATOM:
      offset = build_atom(offset, bound, atom.get.atom);
      break;
    case EXTERNAL:
      offset = build_builtin(offset, bound, atom.get.builtin);
      break;
    case FRAME:
      offset = build_frame(offset, bound, atom.get.frame);
      break;
    case EQUALITY:
    case MEMBERSHIP:
    case SUBCLASS:
      offset = build_term(offset, bound, atom.get.sides[0]);
      offset = build_term(offset, bound, atom.get.sides[1]);
      break;
    default:
      cerr << "[ERROR] Unhandled case at line " << __LINE__ << endl;
      return NULL;
  }
  if (offset != bound) {
    cerr << "[ERROR] Length of atomic is different than expected." << endl;
    return NULL;
  }
  DEBUG("atomic = ", atom);
  return offset;
}

const uint8_t *build_action(const uint8_t *begin, const uint8_t *end, Action &action) {
  if (begin == NULL) {
    return NULL;
  }
  const uint8_t *offset = get_big_endian_int(begin, end, action.type);
  switch (action.type) {
    case ASSERT_FACT:
    case RETRACT_FACT:
    case MODIFY:
      offset = build_atomic(offset, end, action.get.atom);
      break;
    case RETRACT_SLOTS:
      offset = build_term(offset, end, action.get.terms[0]);
      offset = build_term(offset, end, action.get.terms[1]);
      break;
    case RETRACT_OBJECT:
      offset = build_term(offset, end, action.get.term);
      break;
    // TODO case EXECUTE: // got lazy
    default:
      cerr << "[ERROR] Unhandled case at line " << __LINE__ << endl;
      return NULL;
  }
  DEBUG("action = ", action);
  return offset;
}

const uint8_t *build_action_variable_binding(const uint8_t *begin, const uint8_t *end, ActionVariableBinding &binding) {
  if (begin == NULL) {
    return NULL;
  }
  sizeint_t size;
  const uint8_t *offset = get_big_endian_int(begin, end, size);
  const uint8_t *bound = offset + size;
  offset = get_big_endian_int(offset, bound, binding.variable);
  uint8_t newnessflag;
  offset = get_big_endian_int(offset, bound, newnessflag);
  binding.newness = (newnessflag == 0);
  if (!binding.newness) {
    offset = build_atomic(offset, bound, binding.atom);
  }
  if (offset != bound) {
    cerr << "[ERROR] Action variable binding length is different than expected." << endl;
    return NULL;
  }
  DEBUG("binding = ", binding);
  return offset;
}

const uint8_t *build_action_variables(const uint8_t *begin, const uint8_t *end, range_of<ActionVariableBinding> &range_of_actvars) {
  if (begin == NULL) {
    return NULL;
  }
  sizeint_t size;
  vector<ActionVariableBinding> actvars;
  const uint8_t *offset = get_big_endian_int(begin, end, size);
  const uint8_t *bound = offset + size;
  while (offset != NULL && offset != bound) {
    ActionVariableBinding binding;
    offset = build_action_variable_binding(offset, bound, binding);
    actvars.push_back(binding);
  }
  if (offset != bound) {
    cerr << "[ERROR] Action variables declarations length is different than expected." << endl;
    return NULL;
  }
  range_of_actvars.begin = new ActionVariableBinding[actvars.size()];
  range_of_actvars.end = range_of_actvars.begin + actvars.size();
  copy(actvars.begin(), actvars.end(), range_of_actvars.begin);
  DEBUG("range_of_actvars = ", range_of_actvars);
  return offset;
}

const uint8_t *build_action_block(const uint8_t *begin, const uint8_t *end, ActionBlock &action_block) {
  if (begin == NULL) {
    return NULL;
  }
  vector<Action> actions;
  sizeint_t size;
  const uint8_t *offset = get_big_endian_int(begin, end, size);
  const uint8_t *bound = offset + size;
  offset = build_action_variables(offset, bound, action_block.action_variables);
  while (offset != bound) {
    Action action;
    offset = build_action(offset, bound, action);
    actions.push_back(action);
  }
  if (offset != bound) {
    cerr << "[ERROR] Action block length is different than expected." << endl;
    return NULL;
  }
  action_block.actions.begin = new Action[actions.size()];
  action_block.actions.end = action_block.actions.begin + actions.size();
  copy(actions.begin(), actions.end(), action_block.actions.begin);
  DEBUG("action_block = ", action_block);
  return offset;
}

const uint8_t *build_condition(const uint8_t *begin, const uint8_t *end, Condition &condition) {
  if (begin == NULL) {
    return NULL;
  }
  sizeint_t size;
  const uint8_t *offset = get_big_endian_int(begin, end, size);
  const uint8_t *bound = offset + size;
  offset = get_big_endian_int(offset, bound, condition.type);
  switch (condition.type) {
    case ATOMIC: {
      offset = build_atomic(offset, bound, condition.get.atom);
      break;
    }
    case CONJUNCTION:
    case DISJUNCTION: {
      vector<Condition> subformulas;
      while (offset != NULL && offset != bound) {
        Condition subformula;
        offset = build_condition(offset, bound, subformula);
        subformulas.push_back(subformula);
      }
      condition.get.subformulas.begin = new Condition[subformulas.size()];
      condition.get.subformulas.end = condition.get.subformulas.begin + subformulas.size();
      copy(subformulas.begin(), subformulas.end(), condition.get.subformulas.begin);
      break;
    }
    case NEGATION:
    case EXISTENTIAL: {
      condition.get.subformulas.begin = new Condition[1];
      condition.get.subformulas.end = condition.get.subformulas.begin + 1;
      offset = build_condition(offset, bound, *condition.get.subformulas.begin);
      break;
    }
    default: {
      cerr << "[ERROR] Unhandled case " << (int)condition.type << " at line " << __LINE__ << endl;
      return NULL;
    }
  }
  if (offset != bound) {
    cerr << "[ERROR] Condition is a different length than expected." << endl;
    return NULL;
  }
  DEBUG("condition = ", condition);
  return offset;
}

const uint8_t *build_rule(const uint8_t *begin, const uint8_t *end, Rule &rule) {
  if (begin == NULL) {
    return NULL;
  }
  sizeint_t size;
  const uint8_t *offset = get_big_endian_int(begin, end, size);
  const uint8_t *bound = offset + size;
  offset = build_condition(offset, bound, rule.condition);
  offset = build_action_block(offset, bound, rule.action_block);
  if (offset != bound) {
    cerr << "[ERROR] Rule is a different length than expected.";
    return NULL;
  }
  DEBUG("rule = ", rule);
  return offset;
}

#if 1
uint8_t *read_all(const char *filename, size_t &len) {
  deque<uint8_t> bytes;
  ifstream fin(filename);
  while (fin.good()) {
    uint8_t byte = (uint8_t) fin.get();
    if (!fin.eof()) {
      bytes.push_back(byte);
    }
  }
  fin.close();
  len = bytes.size();
  DEBUG("bytes in rule file: ", len);
  uint8_t *p = (uint8_t*)malloc(len);
  copy(bytes.begin(), bytes.end(), p);
  return p;
}
#endif

void load_rules(const char *filename, vector<Rule> &rules) {
  size_t len;
  uint8_t *begin = read_all(filename, len);
  const uint8_t *bytes = begin;
  const uint8_t *end = bytes + len;
  while (bytes != NULL && bytes != end) {
    DEBUG("Building rules at offset ", bytes - begin);
    Rule rule;
    bytes = build_rule(bytes, end, rule);
    rules.push_back(rule);
  }
  if (bytes == NULL) {
    cerr << "[ERROR] An error occurred while loading the rules.";
    rules.clear();
  }
  free(begin);
}

void print_rules(vector<Rule> &rules) {
  vector<Rule>::iterator it = rules.begin();
  for (; it != rules.end(); ++it) {
    cerr << "[RULE] " << *it << endl << endl;
  }
}


////// JOIN PROCESSING //////

typedef vector<constint_t> Tuple;
typedef list<Tuple> Relation;

class Order {
private:
  vector<size_t> order;
public:
  Order() {
    // do nothing
  }
  Order(size_t i1, size_t i2, size_t i3) {
    order.push_back(i1);
    order.push_back(i2);
    order.push_back(i3);
  }
  Order(const vector<size_t> &order)
      : order(order) {
    // do nothing
  }
  bool operator()(const Tuple &t1, const Tuple &t2) const {
    vector<size_t>::const_iterator it = this->order.begin();
    for (; it != this->order.end(); ++it) {
      if (*it >= t1.size()) {
        if (*it < t2.size()) {
          return false;
        }
      } else if (*it >= t2.size()) {
        return true;
      } else if (t1[*it] != t2[*it]) {
        return t1[*it] < t2[*it];
      }
    }
    Tuple::const_iterator cit1 = t1.begin();
    Tuple::const_iterator cit2 = t2.begin();
    while (cit1 != t1.end() && cit2 != t2.end()) {
      if (*cit1 != *cit2) {
        return *cit1 < *cit2;
      }
      ++cit1;
      ++cit2;
    }
    if (cit1 == t1.end()) {
      return cit2 != t2.end();
    }
    return false;
  }
};

typedef set<Tuple, Order> Index;
Index idxspo (Order(0, 1, 2));
Index idxpos (Order(1, 2, 0));
Index idxosp (Order(2, 0, 1));
map<constint_t, Index> atoms;

void load_data(const char *filename) {
  ifstream fin(filename);
  while (fin.good()) {
    Tuple triple(3);
    Tuple::iterator it = triple.begin();
    for (; it != triple.end(); ++it) {
      constint_t c = 0;
      int i;
      for (i = 0; i < sizeof(constint_t); ++i) {
        int b = fin.get();
        if (fin.eof()) {
          if (it != triple.begin() || c != 0) {
            cerr << "[ERROR] Unexpected end of data file.  Only partial data read." << endl;
          }
          return;
        }
        c = (c << 8) | (b & 0xFF);
      }
      *it = c;
    }
    idxspo.insert(triple);
    idxpos.insert(triple);
    idxosp.insert(triple);
  }
}

void minusrel(Relation &intermediate, Relation &negated, Relation &result) {
  cerr << "[ERROR] Negation on non-special formulas is currently unsupported." << endl;
}

bool join(Tuple &t1, Tuple &t2, Tuple &r) {
  if (t1.size() != t2.size()) {
    size_t maxsize = max(t1.size(), t2.size());
    t1.resize(maxsize);
    t2.resize(maxsize);
  }
  Tuple temp(t1.size());
  int i;
  for (i = 0; i < t1.size(); ++i) {
    if (t1[i] == 0 || t1[i] == t2[i]) {
      temp[i] = t2[i];
    } else if (t2[i] == 0) {
      temp[i] = t1[i];
    } else {
      return false;
    }
  }
  r.swap(temp);
  return true;
}

void join(Relation &lhs, Relation &rhs, vector<size_t> &vars, Relation &result) {
  Relation temp;
  Order order(vars);
  lhs.sort(order);
  rhs.sort(order);
  Relation::iterator lit = lhs.begin();
  Relation::iterator rit = rhs.begin();
  while (lit != lhs.end() && rit != rhs.end()) {
    Tuple joined;
    if (join(*lit, *rit, joined)) {
      Relation::iterator it;
      do {
        temp.push_back(joined);
        it = rit;
        for (++it; it != rhs.end() && join(*lit, *it, joined); ++it) {
          temp.push_back(joined);
        }
        ++lit;
      } while (lit != lhs.end() && join(*lit, *rit, joined));
      rit = it;
    } else if (order(*rit, *lit)) {
      ++rit;
    } else {
      ++lit;
    }
  }
  result.swap(temp);
}

#if 0
bool collect_rdf_list(constint_t listid, Relation &relation, set<constint_t> &track) {
  Relation rel;
  if (listid == CONST_RDF_NIL) {
    rel.push_back(Tuple());
    relation.swap(rel);
    return true;
  }
  if (!track.insert(listid).second) {
    cerr << "[WARNING] Cyclical list in the data.  Turning back." << endl;
    rel.push_back(Tuple());
    relation.swap(rel);
    return false;
  }
  bool acyclic = true;
  Tuple minfirstq(3);
  Tuple maxfirstq(3);
  Tuple minrestq(3);
  Tuple maxrestq(3);
  minfirstq[0] = maxfirstq[0] = minrestq[0] = maxrestq[0] = listid;
  minfirstq[1] = maxfirstq[1] = CONST_RDF_FIRST;
  minrestq[1] = maxrestq[1] = CONST_RDF_REST;
  minfirstq[2] = minrestq[2] = 0;
  maxfirstq[2] = maxrestq[2] = CONSTINT_MAX;
  Index::const_iterator lower = idxspo.lower_bound(minrestq);
  Index::const_iterator upper = idxspo.upper_bound(maxrestq);
  for (; lower != upper; ++lower) {
    Relation r;
    acyclic = collect_rdf_list(lower->at(2), r, track) && acyclic;
    rel.splice(rel.end(), r);
  }
  lower = idxspo.lower_bound(minfirstq);
  upper = idxspo.upper_bound(minrestq);
  for (; lower != upper; ++lower) {
    Relation::iterator it = rel.begin();
    for (; it != rel.end(); ++it) {
      it->push_front(lower->at(2));
    }
  }
  relation.swap(rel);
  return acyclic;
}

bool collect_rdf_list(constint_t listid, Relation &relation) {
  set<constint_t> track;
  return collect_rdf_list(listid, relation, track);
}
#endif

bool special(Condition &condition, Relation &intermediate, Relation &filtered) {
  if (condition.type != ATOMIC) {
    return false;
  }
  switch (condition.get.atom.type) {
    case ATOM:
    case MEMBERSHIP:
    case SUBCLASS:
    case FRAME:
      return false;
    case EQUALITY: {
      Term *lhs = &(condition.get.atom.get.sides[0]);
      Term *rhs = &(condition.get.atom.get.sides[1]);
      if (lhs->type == VARIABLE && rhs->type == CONSTANT) {
        swap(lhs, rhs);
      }
      if (rhs->type == CONSTANT) {
        if (lhs->get.constant != rhs->get.constant) {
          Relation empty;
          filtered.swap(empty);
        }
      } else if (lhs->type == CONSTANT) {
        Relation result;
        Relation::iterator it = intermediate.begin();
        for (; it != intermediate.end(); ++it) {
          if (it->size() <= rhs->get.variable) {
            it->resize(rhs->get.variable + 1);
          }
          if (it->at(rhs->get.variable) == 0) {
            result.push_back(*it);
            Tuple &t = result.back();
            t[rhs->get.variable] = lhs->get.constant;
          } else if (it->at(rhs->get.variable) == lhs->get.constant) {
            result.push_back(*it);
          }
        }
        filtered.swap(result);
      } else {
        varint_t maxvar = max(lhs->get.variable, rhs->get.variable);
        Relation result;
        Relation::iterator it = intermediate.begin();
        for (; it != intermediate.end(); ++it) {
          if (it->size() <= maxvar) {
            it->resize(maxvar + 1);
          }
          constint_t c1 = it->at(lhs->get.variable);
          constint_t c2 = it->at(rhs->get.variable);
          if (c1 == 0 && c2 == 0) {
            cerr << "[ERROR] Please make sure equality statements occur to the right of atomic formulas that bind a variable." << endl;
            return false;
          }
          if (c1 == 0) {
            result.push_back(*it);
            Tuple &t = result.back();
            t[lhs->get.variable] = c2;
          } else if (c2 == 0) {
            result.push_back(*it);
            Tuple &t = result.back();
            t[rhs->get.variable] = c1;
          } else if (c1 == c2) {
            result.push_back(*it);
          }
        }
        filtered.swap(result);
      }
      return true;
    }
    case EXTERNAL: {
      break; // just handle outside the messy switch statement
    }
    default: {
      cerr << "[ERROR] Unhandled case " << (int)condition.get.atom.type << " at line " << __LINE__ << endl;
      return false;
    }
  }

  ///// HANDLE BUILTIN (EXTERNAL) /////
  Builtin &builtin = condition.get.atom.get.builtin;
  switch (builtin.predicate) {
    case BUILTIN_PRED_LIST_CONTAINS: {
      if (builtin.arguments.end - builtin.arguments.begin != 2) {
        cerr << "[ERROR] Invalid use of list contains builtin, which should have exactly two arguments, but instead found " << builtin << endl;
        return false;
      }
      Term *arg1 = builtin.arguments.begin;
      Term *arg2 = arg1 + 1;
      if (arg1->type != LIST) {
        cerr << "[ERROR] First argument of list contains builtin must be a list, but instead found " << *arg1 << endl;
        return false;
      }
      switch (arg2->type) {
        case FUNCTION:
          cerr << "[ERROR] Functions not supported." << endl;
          return false;
        case LIST:
          cerr << "[ERROR] Currently not supporting nested lists." << endl;
          return false;
        case CONSTANT: {
          Term *t = arg1->get.termlist.begin;
          for (; t != arg1->get.termlist.end; ++t) {
            if (t->type == CONSTANT && t->get.constant == arg2->get.constant) {
              return true;
            }
          }
          Relation empty;
          filtered.swap(empty);
          return true;
        }
        case VARIABLE:
          break; // just handle outside messy switch statement
        default:
          cerr << "[ERROR] Unhandled case " << (int)builtin.predicate << " at line " << __LINE__ << endl;
          return false;
      }
      Relation results;
      Relation::iterator it = intermediate.begin();
      for (; it != intermediate.end(); ++it) {
        if (it->size() <= arg2->get.variable) {
          it->resize(arg2->get.variable + 1);
        }
        constint_t c = it->at(arg2->get.variable);
        if (c == 0) {
          Term *t = arg1->get.termlist.begin;
          for (; t != arg1->get.termlist.end; ++t) {
            if (t->type == CONSTANT) {
              results.push_back(*it);
              results.back().at(arg2->get.variable) = t->get.constant;
            }
          }
        } else {
          Term *t = arg1->get.termlist.begin;
          for (; t != arg1->get.termlist.end; ++t) {
            if (t->type == CONSTANT && t->get.constant == c) {
              results.push_back(*it);
              break;
            }
          }
        }
      }
      filtered.swap(results);
      return true;
    }
    default: {
      cerr << "[ERROR] Builtin predicate " << (int)builtin.predicate << " is unsupported." << endl;
      return false;
    }
  }
}

void query_atom(Atom &atom, set<varint_t> &allvars, Relation &results) {
  // just doing a scan
  // in the general case, this is terribly inefficient
  // but since I will need it only for dealing with rdf:Lists
  // which are presumably quite short, it should be okay
  // ... hmmm... not so sure about this TODO
  set<varint_t> newvars;
  varint_t maxvar = 0;
  Term *term = atom.arguments.begin;
  for (; term != atom.arguments.end; ++term) {
    if (term->type == VARIABLE) {
      newvars.insert(term->get.variable);
      maxvar = max(maxvar, term->get.variable);
    } else if (term->type != CONSTANT) {
      cerr << "[ERROR] Not handling lists or functions in atoms." << endl;
      return;
    }
  }
  Index &base = atoms[atom.predicate];
  Relation temp;
  Index::iterator it = base.begin();
  term = atom.arguments.begin;
  size_t max = atom.arguments.end - atom.arguments.begin;
  for (; it != base.end(); ++it) {
    Tuple result(maxvar+1);
    size_t i;
    for (i = 0; i < max; ++i) {
      if (term[i].type == VARIABLE) {
        result[term[i].get.variable] = it->at(i);
      } else if (term[i].type == CONSTANT) {
        if (it->at(i) != term[i].get.constant) {
          break;
        }
      }
    }
    if (i == max) {
      temp.push_back(result);
    }
  }
  allvars.insert(newvars.begin(), newvars.end());
  results.swap(temp);
}

void query(Atomic &atom, set<varint_t> &allvars, Relation &results) {
  switch (atom.type) {
    case ATOM:
      query_atom(atom.get.atom, allvars, results);
      return;
    case MEMBERSHIP:
    case SUBCLASS:
    case EQUALITY:
      return; // only concerned with triples for data
    case EXTERNAL:
      cerr << "[ERROR] This should never happen, at line " << __LINE__ << endl;
      return;
    case FRAME:
      break; // just handle outside this messy switch
    default:
      cerr << "[ERROR] Unhandled case " << (int) atom.type << " at line " << __LINE__ << endl;
      return;
  }
  Relation intermediate;
  Term subj = atom.get.frame.object;
  if (subj.type == LIST) {
    return; // no lists (real lists, not rdf:Lists) in RDF
  }
  if (subj.type == FUNCTION) {
    cerr << "[ERROR] Functions are currently unsupported." << endl;
    return;
  }
  varint_t maxvar = 0;
  Tuple mintriple(3);
  Tuple maxtriple(3);
  if (subj.type == CONSTANT) {
    mintriple[0] = maxtriple[0] = subj.get.constant;
  } else {
    mintriple[0] = 0;
    maxtriple[0] = CONSTINT_MAX;
    maxvar = subj.get.variable;
    allvars.insert(subj.get.variable);
  }
  pair<Term, Term> *slot = atom.get.frame.slots.begin;
  for (; slot != atom.get.frame.slots.end; ++slot) {
    Term pred = slot->first;
    Term obj = slot->second;
    if (pred.type == LIST || obj.type == LIST) {
      continue; // no lists (real lists, not rdf:Lists) in RDF
    }
    if (pred.type == FUNCTION || obj.type == FUNCTION) {
      cerr << "[ERROR] Functions are currently unsupported." << endl;
      return;
    }
    set<varint_t> newvars;
    if (subj.type == VARIABLE) {
      newvars.insert(subj.get.variable);
    }
    if (pred.type == CONSTANT) {
      mintriple[1] = maxtriple[1] = pred.get.constant;
    } else {
      mintriple[1] = 0;
      maxtriple[1] = CONSTINT_MAX;
      maxvar = max(maxvar, pred.get.variable);
      newvars.insert(pred.get.variable);
    }
    if (obj.type == CONSTANT) {
      mintriple[2] = maxtriple[2] = obj.get.constant;
    } else {
      mintriple[2] = 0;
      maxtriple[2] = CONSTINT_MAX;
      maxvar = max(maxvar, obj.get.variable);
      newvars.insert(obj.get.variable);
    }
    int idx = (subj.type == CONSTANT ? 0x4 : 0x0) |
              (pred.type == CONSTANT ? 0x2 : 0x0) |
              ( obj.type == CONSTANT ? 0x1 : 0x0);
    Index::const_iterator begin, end;
    switch (idx) {
      case 0x0:
      case 0x4:
      case 0x6:
      case 0x7: // SPO
        begin = idxspo.lower_bound(mintriple);
        end = idxspo.upper_bound(maxtriple);
        break;
      case 0x2:
      case 0x3: // POS
        begin = idxpos.lower_bound(mintriple);
        end = idxpos.upper_bound(maxtriple);
        break;
      case 0x1:
      case 0x5: // OSP
        begin = idxosp.lower_bound(mintriple);
        end = idxosp.upper_bound(maxtriple);
        break;
      default:
        cerr << "[ERROR] Unhandled case " << hex << idx << " at line " << dec << __LINE__ << endl;
        return;
    }
    // TODO the following loop could probably be more efficient
    Relation selection;
    for (; begin != end; ++begin) {
      selection.push_back(Tuple(maxvar + 1));
      Tuple &result = selection.back();
      if (subj.type == VARIABLE) {
        result[subj.get.variable] = begin->at(0);
      }
      if (pred.type == VARIABLE) {
        result[pred.get.variable] = begin->at(1);
      }
      if (obj.type == VARIABLE) {
        result[obj.get.variable] = begin->at(2);
      }
    }
    if (slot == atom.get.frame.slots.begin) {
      intermediate.swap(selection);
    } else {
      vector<size_t> joinvars (allvars.size() + newvars.size());
      vector<size_t>::iterator jit = set_intersection(allvars.begin(),
          allvars.end(), newvars.begin(), newvars.end(), joinvars.begin());
      joinvars.resize(jit - joinvars.begin());
      join(intermediate, selection, joinvars, intermediate);
    }
    allvars.insert(newvars.begin(), newvars.end());
  }
  results.swap(intermediate);
}

void query(Condition &condition, set<varint_t> &allvars, Relation &results) {
  deque<void*> negated;
  Relation intermediate;
  switch (condition.type) {
    case ATOMIC: {
      query(condition.get.atom, allvars, results);
      return;
    }
    case CONJUNCTION: {
      Condition *subformula = condition.get.subformulas.begin;
      for (; subformula != condition.get.subformulas.end; ++subformula) {
        if (subformula->type == NEGATION) {
          negated.push_back((void*)subformula->get.subformulas.begin);
          continue;
        }
        Relation subresult;
        set<varint_t> newvars;
        // TODO vvv this won't work right if a non-special query
        // has not been performed first
        if (special(*subformula, intermediate, subresult)) {
          if (subformula == condition.get.subformulas.begin) {
            cerr << "[ERROR] Must have non-special query at beginning of conjunction." << endl;
          }
          intermediate.swap(subresult);
          continue;
        }
        query(*subformula, newvars, subresult);
        if (subformula == condition.get.subformulas.begin) {
          intermediate.swap(subresult);
        } else {
          vector<size_t> joinvars(allvars.size() + newvars.size());
          vector<size_t>::iterator jit = set_intersection(allvars.begin(),
              allvars.end(), newvars.begin(), newvars.end(), joinvars.begin());
          joinvars.resize(jit - joinvars.begin());
          join(intermediate, subresult, joinvars, intermediate);
        }
        allvars.insert(newvars.begin(), newvars.end());
      }
      break;
    }
    case DISJUNCTION: {
      cerr << "[ERROR] Disjunction is not supported." << endl;
      return;
    }
    case EXISTENTIAL: {
      // assuming all quantified variables are uniquely named in the scope of the rule
      Condition *subformula = condition.get.subformulas.begin;
      for (; subformula != condition.get.subformulas.end; ++subformula) {
        if (subformula->type == NEGATION) {
          negated.push_back((void*)subformula->get.subformulas.begin);
        }
        Relation subresult;
        query(*subformula, allvars, subresult);
        intermediate.splice(intermediate.end(), subresult);
      }
      break;
    }
    case NEGATION: {
      cerr << "[ERROR] This should never happen.  query should not be called directly on negated formulas." << endl;
      return;
    }
    default: {
      cerr << "[ERROR] Unhandled case " << (int)condition.type << " on line " << __LINE__ << endl;
      return;
    }
  }
// TODO deal with negation later, if at all
#if 1
  deque<void*>::iterator it = negated.begin();
  for (; !intermediate.empty() && it != negated.end(); ++it) {
    Relation negresult;
    Condition *cond = (Condition*) *it;
    if (special(*cond, intermediate, negresult)) {
      intermediate.sort();
      negresult.sort();
      Relation leftover(intermediate.size());
      Relation::iterator iit = set_difference(intermediate.begin(),
          intermediate.end(), negresult.begin(), negresult.end(),
          leftover.begin());
      Relation newinter;
      newinter.splice(newinter.end(), leftover,
                      leftover.begin(), iit);
      intermediate.swap(newinter);
      continue;
    }
    query(*((Condition*)(*it)), allvars, negresult);
    minusrel(intermediate, negresult, intermediate);
  }
#endif
  results.swap(intermediate);
}

void act(Atom &atom, Relation &results) {
  DEBUG("Act: atom = ", atom);
  Index &index = atoms[atom.predicate];
  Relation::iterator result = results.begin();
  size_t max = atom.arguments.end - atom.arguments.begin;
  for (; result != results.end(); ++result) {
    Tuple tuple(max);
    size_t i;
    for (i = 0; i < max; ++i) {
      if (atom.arguments.begin[i].type == CONSTANT) {
        tuple[i] = atom.arguments.begin[i].get.constant;
        DEBUG("part ", tuple[i]);
      } else if (atom.arguments.begin[i].type == VARIABLE) {
        tuple[i] = result->at(atom.arguments.begin[i].get.variable);
        DEBUG("part ", tuple[i]);
      } else {
        cerr << "[ERROR] Lists and/or function in target atom are not currently supported." << endl;
        return;
      }
    }
    DEBUG("inserting (ignore this ->", max);
    index.insert(tuple);
  }
}

void act(ActionBlock &action_block, Relation &results, Index &assertions, Index &retractions) {
  if (action_block.action_variables.begin != action_block.action_variables.end) {
    cerr << "[ERROR] Action variables are unsupported." << endl;
    return;
  }
  Action *action = action_block.actions.begin;
  for (; action != action_block.actions.end; ++action) {
    switch (action->type) {
      case ASSERT_FACT:
      case RETRACT_FACT: {
        if (action->get.atom.type == ATOM) {
          if (action->type == RETRACT_FACT) {
            cerr << "[ERROR] Retraction of atoms is currently unsupported." << endl;
            return;
          }
          act(action->get.atom.get.atom, results);
          continue;
        }
        if (action->get.atom.type != FRAME) {
          cerr << "[ERROR] For now, supporting only assertion of frames/triples." << endl;
          return;
        }
        Frame frame = action->get.atom.get.frame;
        Tuple triple(3);
        if (frame.object.type == LIST || frame.object.type == FUNCTION) {
          cerr << "[ERROR] Not supporting lists or functions in action target." << endl;
          return;
        }
        if (frame.object.type == CONSTANT) {
          triple[0] = frame.object.get.constant;
        }
        pair<Term, Term> *slot = frame.slots.begin;
        for (; slot != frame.slots.end; ++slot) {
          if (slot->first.type == LIST || slot->first.type == FUNCTION ||
              slot->second.type == LIST || slot->second.type == FUNCTION) {
            cerr << "[ERROR] Not supporting lists or function in action target." << endl;
            return;
          }
          if (slot->first.type == CONSTANT) {
            triple[1] = slot->first.get.constant;
          }
          if (slot->second.type == CONSTANT) {
            triple[2] = slot->second.get.constant;
          }
          if (!results.empty() && frame.object.type == CONSTANT && slot->first.type == CONSTANT && slot->second.type == CONSTANT) {
            if (action->type == ASSERT_FACT) {
              assertions.insert(triple);
            } else {
              retractions.insert(triple);
            }
            continue;
          }
          Relation::iterator tuple = results.begin();
          for (; tuple != results.end(); ++tuple) {
            if (frame.object.type == VARIABLE) {
              triple[0] = tuple->at(frame.object.get.variable);
            }
            if (slot->first.type == VARIABLE) {
              triple[1] = tuple->at(slot->first.get.variable);
            }
            if (slot->second.type == VARIABLE) {
              triple[2] = tuple->at(slot->second.get.variable);
            }
            if (action->type == ASSERT_FACT) {
              assertions.insert(triple);
            } else {
              retractions.insert(triple);
            }
          }
        }
        return;
      }
      default: {
        cerr << "[ERROR] Currently supporting on ASSERT_FACT and RETRACT_FACT actions." << endl;
        return;
      }
    }
  }
}

// infer until fixpoint, which may not be appropriate in the presence of retraction
void infer(vector<Rule> &rules) {
  bool changed = true;
  map<constint_t, size_t> sizes;
  map<constint_t, Index>::const_iterator atomit = atoms.begin();
  for (; atomit != atoms.end(); ++atomit) {
    sizes[atomit->first] = atomit->second.size();
  }
  while (changed) {
    changed = false;
    Index assertions (Order(0, 1, 2));
    Index retractions (Order(0, 1, 2));
    int rulecount = 0;
    vector<Rule>::iterator rule = rules.begin();
    for (; rule != rules.end(); ++rule) {
      Relation results;
      set<varint_t> allvars;
      query(rule->condition, allvars, results);
      act(rule->action_block, results, assertions, retractions);
    }
    Index::iterator it = retractions.begin();
    for (; it != retractions.end(); ++it) {
      if (idxspo.erase(*it) > 0) {
        idxpos.erase(*it);
        idxosp.erase(*it);
        changed = true;
      }
    }
    it = assertions.begin();
    for (; it != assertions.end(); ++it) {
      if (idxspo.insert(*it).second) {
        idxpos.insert(*it);
        idxosp.insert(*it);
        changed = true;
      }
    }
    atomit = atoms.begin();
    for (; atomit != atoms.end(); ++atomit) {
      changed = changed || sizes[atomit->first] != atomit->second.size();
      sizes[atomit->first] = atomit->second.size();
    }
  }
}

#define FOR_HUMAN_EYES 0
void print_data() {
  Index::iterator it = idxspo.begin();
  for (; it != idxspo.end(); ++it) {
    Tuple::const_iterator tit = it->begin();
    for (; tit != it->end(); ++tit) {
#if !FOR_HUMAN_EYES
      size_t i;
      for (i = 0; i < sizeof(constint_t); ++i) {
        uint8_t byte = (uint8_t) (((*tit) >> ((sizeof(constint_t) - i - 1) << 3)) & 0xFF);
        cout.write((const char*)&byte, sizeof(uint8_t));
      }
#else
      cout << hexify(*tit) << ' ';
#endif
    }
#if FOR_HUMAN_EYES
    cout << endl;
#endif
  }
  if (!atoms[CONST_RIF_ERROR].empty()) {
    cerr << "INCONSISTENT" << endl;
  }
}












int main(int argc, char **argv) {
  vector<Rule> rules;
  load_rules(argv[1], rules);
  //print_rules(rules);
  load_data(argv[2]);
  infer(rules);
  print_data();
  return 0;
}
