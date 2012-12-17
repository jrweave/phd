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

#include <algorithm>
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
#include "rif/RIFTerm.h"
#include "sat/SATFormula.h"
#include "util/timing.h"

#define SAFEPICK

/* QUICKVERIFY checks to see if the fixed replication scheme and given rules
 * meet the conditions.  QUICKCHECK is sufficient, but not necessary, for
 * meeting the conditions.
 */
#ifdef QUICKVERIFY
#define KEEP_SAFE 1
#define KEEP_RISKY 1
#define TRASH_RISKY 0
#define PARTITION_RISKY 0
#define PARTITION_PATTERNS 0
#endif

/* VERIFY checks to see if the fixed replication scheme and given rules meet
 * the conditions.  Unlike QUICKCHECK, CHECK is both necessary and sufficient
 * for meeting the conditions.
 */
#ifdef VERIFY
#define KEEP_SAFE 1
#define KEEP_RISKY 1
#define TRASH_RISKY 0
#define PARTITION_RISKY 1
#define PARTITION_PATTERNS 0
#endif

/* QUICKPICK is the fastest way to pick a subset of the rules for which the
 * conditions are met.  However, it may sacrifice semantics too easily.
 */
#ifdef QUICKPICK
#define KEEP_SAFE 1
#define KEEP_RISKY 0
#define TRASH_RISKY 1
#define PARTITION_RISKY 0
#define PARTITION_PATTERNS 0
#endif

/* SAFEPICK is slower than QUICKPICK but faster than PICK and SLOWPICK.
 * At this point, though, running on large numbers of rules can become
 * impractical.
 */
#ifdef SAFEPICK
#define KEEP_SAFE 1
#define KEEP_RISKY 0
#define TRASH_RISKY 0
#define PARTITION_RISKY 1
#define PARTITION_PATTERNS 0
#endif

/* PICK is slow even on small numbers of rules, but may preserve more
 * semantics.
 */
#ifdef PICK
#define KEEP_SAFE 0
#define KEEP_RISKY 0
#define TRASH_RISKY 0
#define PARTITION_RISKY 0
#define PARTITION_PATTERNS 0
#endif

/* SLOWPICK will probably never finish, but it preserves all semantics.
 */
#ifdef SLOWPICK
#define KEEP_SAFE 0
#define KEEP_RISKY 0
#define TRASH_RISKY 0
#define PARTITION_RISKY 1
#define PARTITION_PATTERNS 0
#endif

#ifdef OPTPICK
#define KEEP_SAFE 0
#define KEEP_RISKY 0
#define TRASH_RISKY 0
#define PARTITION_RISKY 1
#define PARTITION_PATTERNS 1
#endif

#if KEEP_RISKY && TRASH_RISKY
#error Only one of KEEP_RISKY and TRASH RISKY can be set.
#endif

#define FILELINE //cerr << __FILE__ << ':' << __LINE__ << endl

using namespace ptr;
using namespace rif;
using namespace sat;
using namespace std;

typedef bool (*TermLTFunc)(const RIFTerm &, const RIFTerm &);

typedef set<RIFTerm,
            bool (*)(const RIFTerm &, const RIFTerm &)>
        TSet;
#define TSET(varname) TSet varname(RIFTerm::cmplt0)

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

ostream &operator<<(ostream &stream, const RIFAction &act) {
  DPtr<uint8_t> *str = act.toUTF8String();
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

template<typename T, typename CMP>
void set_difference(const set<T, CMP> &lhs, const set<T, CMP> &rhs, set<T, CMP> &diff) {
  typename set<T, CMP>::const_iterator it = lhs.begin();
  for (; it != lhs.end(); ++it) {
    if (rhs.count(*it) <= 0) {
      diff.insert(*it);
    }
  }
}

template<typename T, typename CMP>
void set_intersection(const set<T, CMP> &lhs, const set<T, CMP> &rhs, set<T, CMP> &inter) {
  typename set<T, CMP>::const_iterator it = lhs.begin();
  for (; it != lhs.end(); ++it) {
    if (rhs.count(*it) > 0) {
      inter.insert(*it);
    }
  }
}

template<typename T, typename CMP>
void set_union(const set<T, CMP> &lhs, const set<T, CMP> &rhs, set<T, CMP> &uni) {
  uni.insert(lhs.begin(), lhs.end());
  uni.insert(rhs.begin(), rhs.end());
}

class Restriction {
private:
  bool allow;
  TSet terms;
public:
  Restriction()
      : allow(false), terms(TSet(RIFTerm::cmplt0)) {
    // do nothing
  }
  Restriction(const bool allow)
      : allow(allow), terms(TSet(RIFTerm::cmplt0)) {
    // do nothing
  }
  Restriction(const bool allow, const TSet &values)
      : allow(allow), terms(values) {
    TSet::const_iterator it = this->terms.begin();
    for (; it != this->terms.end(); ++it) {
      if (!it->isGround()) {
        cerr << "[ERROR] Restriction terms must all be ground, but found: " << *it << endl;
        THROW(TraceableException, "Restrictions terms must all be ground.");
      }
    }
  }
  Restriction(TSet &values, const bool allow)
      : allow(allow), terms(TSet(RIFTerm::cmplt0)) {
    this->terms.swap(values);
  }
  Restriction(const Restriction &copy)
      : allow(copy.allow), terms(copy.terms) {
    // do nothing
  }
  ~Restriction() {
    // do nothing
  }
  bool empty() const {
    return this->allow && this->terms.empty();
  }
  bool full() const {
    return !this->allow && this->terms.empty();
  }
  bool allows() const {
    return this->allow;
  }
  bool disallows() const {
    return !this->allow;
  }
  const TSet &getTerms() const {
    return this->terms;
  }
  bool addTerm(const RIFTerm &term) {
    if (!term.isGround()) {
      cerr << "[ERROR] term must be ground, but found " << term << endl;
      THROW(TraceableException, "term must be ground.");
    }
    return this->terms.insert(term).second;
  }
  bool removeTerm(const RIFTerm &term) {
    return this->terms.erase(term) > 0;
  }
  Restriction minus(const Restriction &rhs) const {
    TSET(newterms);
    if (this->allow) {
      if (rhs.allow) {
        set_difference(this->terms, rhs.terms, newterms);
        return Restriction(newterms, true);
      } else {
        set_intersection(this->terms, rhs.terms, newterms);
        return Restriction(newterms, true);
      }
    } else {
      if (rhs.allow) {
        set_union(this->terms, rhs.terms, newterms);
        return Restriction(newterms, false);
      } else {
        set_difference(rhs.terms, this->terms, newterms);
        return Restriction(newterms, true);
      }
    }
  }
  Restriction intersectWith(const Restriction &rhs) const {
    TSET(newterms);
    if (this->allow) {
      if (rhs.allow) {
        set_intersection(this->terms, rhs.terms, newterms);
        return Restriction(newterms, true);
      } else {
        set_difference(this->terms, rhs.terms, newterms);
        return Restriction(newterms, true);
      }
    } else {
      if (rhs.allow) {
        set_difference(rhs.terms, this->terms, newterms);
        return Restriction(newterms, true);
      } else {
        set_union(this->terms, rhs.terms, newterms);
        return Restriction(newterms, false);
      }
    }
  }
  Restriction unionWith(const Restriction &rhs) const {
    TSET(newterms);
    if (this->allow) {
      if (rhs.allow) {
        set_union(this->terms, rhs.terms, newterms);
        return Restriction(newterms, true);
      } else {
        set_difference(rhs.terms, this->terms, newterms);
        return Restriction(newterms, false);
      }
    } else {
      if (rhs.allow) {
        set_difference(this->terms, rhs.terms, newterms);
        return Restriction(newterms, false);
      } else {
        set_intersection(this->terms, rhs.terms, newterms);
        return Restriction(newterms, false);
      }
    }
  }
  bool subsetOf(const Restriction &rhs) const {
    return this->minus(rhs).empty();
  }
  bool properSubsetOf(const Restriction &rhs) const {
    return this->subsetOf(rhs) && !rhs.subsetOf(*this);
  }
  bool equivalentTo(const Restriction &rhs) const {
    return this->subsetOf(rhs) && rhs.subsetOf(*this);
  }
  bool overlapsWith(const Restriction &rhs) const {
    return !this->intersectWith(rhs).empty();
  }
  bool contains(const RIFTerm &term) const {
    if (this->allow) {
      return this->terms.count(term) > 0;
    } else {
      return this->terms.count(term) <= 0;
    }
  }
  void swap(Restriction &r) {
    this->terms.swap(r.terms);
    std::swap(this->allow, r.allow);
  }
  bool operator<(const Restriction &rhs) const {
    if (this->allow != rhs.allow) {
      return rhs.allow;
    }
    if (this->terms.size() != rhs.terms.size()) {
      return this->terms.size() < rhs.terms.size();
    }
    TSet::const_iterator it1 = this->terms.begin();
    TSet::const_iterator it2 = rhs.terms.begin();
    for (; it1 != this->terms.end(); ++it1) {
      int c = RIFTerm::cmp(*it1, *it2);
      if (c != 0) {
        return c < 0;
      }
      ++it2;
    }
    return false;
  }
  bool operator<=(const Restriction &rhs) const {
    return !(rhs < *this);
  }
  bool operator==(const Restriction &rhs) const {
    return !(*this < rhs) && !(rhs < *this);
  }
  bool operator>=(const Restriction &rhs) const {
    return !(*this < rhs);
  }
  bool operator>(const Restriction &rhs) const {
    return rhs < *this;
  }
  bool operator!=(const Restriction &rhs) const {
    return *this < rhs || rhs < *this;
  }
  Restriction &operator=(const Restriction &rhs) {
    TSet newterms(rhs.terms);
    this->terms.swap(newterms);
    this->allow = rhs.allow;
    return *this;
  }
};

typedef map<RIFTerm,
            size_t,
            bool (*)(const RIFTerm &, const RIFTerm &)>
        RMap;

typedef map<RIFTerm,
            Restriction,
            bool (*)(const RIFTerm &, const RIFTerm &)>
        RRMap;

class Pattern;

ostream &operator<<(ostream &stream, const Pattern &pattern);

class Pattern {
private:
  RIFAtomic atom;
  map<size_t, Restriction> restrictions;
  map<size_t, size_t> redirects;
  RMap index;
  bool forced_empty;
  Restriction &lookup(const size_t i) {
    map<size_t, Restriction>::iterator rit = this->restrictions.find(i);
    if (rit == this->restrictions.end()) {
      map<size_t, size_t>::const_iterator it = this->redirects.find(i);
      if (it == this->redirects.end()) {
        THROW(TraceableException, "Illegal state.  This should never happen.");
      }
      rit = this->restrictions.find(it->second);
      if (rit == this->restrictions.end()) {
        THROW(TraceableException, "Illegal state.  This should never happen.");
      }
    }
    return rit->second;
  }
  const Restriction &lookupc(const size_t i) const {
    map<size_t, Restriction>::const_iterator rit = this->restrictions.find(i);
    if (rit == this->restrictions.end()) {
      map<size_t, size_t>::const_iterator it = this->redirects.find(i);
      if (it == this->redirects.end()) {
        THROW(TraceableException, "Illegal state.  This should never happen.");
      }
      rit = this->restrictions.find(it->second);
      if (rit == this->restrictions.end()) {
        THROW(TraceableException, "Illegal state.  This should never happen.");
      }
    }
    return rit->second;
  }
public:
  Pattern(const RIFAtomic &atom)
      : atom(atom), index(RMap(RIFTerm::cmplt0)), forced_empty(false) {
    if (atom.getType() == FRAME && atom.getArity() != 3) {
      cerr << "[ERROR] Only handling frames with exactly one slot, not " << atom << endl;
      THROW(TraceableException, "Only handling frames with exactly one slot.");
    }
    size_t i;
    for (i = 1; i <= atom.getArity(); ++i) {
      RIFTerm t = atom.getPart(i);
      if (t.getType() == FUNCTION && t.isGround()) {
        cerr << "[ERROR] Ground function terms like " << t << " should be replaced with their constant values." << endl;
        THROW(TraceableException, "Ground function terms should be replaced with their constant values.");
      }
      RMap::iterator it = this->index.find(t);
      if (it == this->index.end()) {
        this->index.insert(pair<RIFTerm, size_t>(t, i));
        if (t.isGround()) {
          Restriction r(true);
          r.addTerm(t);
          this->restrictions[i].swap(r);
        } else {
          Restriction r(false);
          this->restrictions[i].swap(r);
        }
      } else {
        this->redirects[i] = it->second;
      }
    }
  }
  Pattern(const Pattern &copy)
      : atom(copy.atom), restrictions(copy.restrictions),
        redirects(copy.redirects), index(copy.index),
        forced_empty(copy.forced_empty) {
    // do nothing
  }
  ~Pattern() {
    // do nothing
  }
  RIFAtomic getAtom() const {
    return this->atom;
  }
  const map<size_t, Restriction> &getRestrictions() const {
    return this->restrictions;
  }
  const map<size_t, size_t> &getRedirects() const {
    return this->redirects;
  }
  const RMap &getIndex() const {
    return this->index;
  }
  const void intersectRestrictions(RRMap &restrs) const {
    RMap::const_iterator it = this->index.begin();
    for (; it != this->index.end(); ++it) {
      const Restriction &r = this->lookupc(it->second);
      RRMap::iterator it2 = restrs.find(it->first);
      if (it2 == restrs.end()) {
        restrs.insert(pair<RIFTerm, Restriction>(it->first, r));
      } else {
        it2->second = it2->second.intersectWith(r);
      }
    }
  }
  bool empty() const {
    if (this->forced_empty) {
      return true;
    }
    map<size_t, Restriction>::const_iterator it = this->restrictions.begin();
    for (; it != this->restrictions.end(); ++it) {
      if (it->second.empty()) {
        return true;
      }
    }
    RMap::const_iterator it2 = this->index.begin();
    for (; it2 != this->index.end(); ++it2) {
      if (it2->first.isGround() && !this->lookupc(it2->second).contains(it2->first)) {
        return true;
      }
    }
    return false;
  }
  bool isGround() const {
    if (this->empty()) {
      return true;
    }
    map<size_t, Restriction>::const_iterator it = this->restrictions.begin();
    for (; it != this->restrictions.end(); ++it) {
      if (it->second.disallows() || it->second.getTerms().size() > 1) {
        return false;
      }
    }
    return true;
  }
  void restrictTerm(const RIFTerm &term, const Restriction &restr) {
    if (this->empty()) {
      this->forced_empty = true;
      return;
    }
    RMap::iterator it = this->index.find(term);
    if (it == this->index.end()) {
      return;
    }
    Restriction &r = this->restrictions[it->second];
    r = r.intersectWith(restr);
  }
  void restrictTerms(const RRMap &restrs) {
    if (this->empty()) {
      this->forced_empty = true;
      return;
    }
    RRMap::const_iterator it = restrs.begin();
    for (; it != restrs.end(); ++it) {
      this->restrictTerm(it->first, it->second);
    }
  }
  void restrictTerm(const RIFTerm &term, const bool allow, const RIFTerm &val) {
    if (!val.isGround()) {
      cerr << "[ERROR] Terms may be restricted only to ground terms, not " << val << endl;
      THROW(TraceableException, "Terms may be restricted only to ground terms.");
    }
    if (val.getType() == FUNCTION) {
      cerr << "[ERROR] Ground function terms like " << val << " should be replaced with their constant values." << endl;
      THROW(TraceableException, "Ground function terms should be replaced with their constant values.");
    }
    if (this->empty()) {
      this->forced_empty = true;
      return;
    }
    RMap::iterator it = this->index.find(term);
    if (it == this->index.end()) {
      return;
    }
    Restriction &r = this->restrictions[it->second];
    if (r.allows() == allow) {
      r.addTerm(val);
    } else {
      TSET(tset);
      tset.insert(term);
      Restriction r2(allow, tset);
      r = r.intersectWith(r2);
    }
  }
  Pattern intersectWith(const Pattern &rhs) const {
    if (this->empty()) {
      return *this;
    }
    if (rhs.empty()) {
      Pattern p(this->atom);
      p.forced_empty = true;
      return p;
    }
    if (this->atom.getType() != rhs.atom.getType() ||
        this->atom.getArity() != rhs.atom.getArity()) {
      Pattern p(this->atom);
      p.forced_empty = true;
      return p;
    }
    if (this->atom.getType() == ATOM &&
        !this->atom.getPred().equals(rhs.atom.getPred())) {
      Pattern p(this->atom);
      p.forced_empty = true;
      return p;
    }
    Pattern p(*this);
    size_t i;
    for (i = 1; i <= p.atom.getArity(); ++i) {
      Restriction &r1 = p.lookup(i);
      const Restriction &r2 = rhs.lookupc(i);
      r1 = r1.intersectWith(r2);
      if (r1.empty()) {
        p.forced_empty = true;
        return p;
      }
    }
    multimap<size_t, size_t> reverse_redirects;
    map<size_t, size_t>::const_iterator redit = p.redirects.begin();
    for (; redit != p.redirects.end(); ++redit) {
      reverse_redirects.insert(pair<size_t, size_t>(redit->second, redit->first));
    }
    for (redit = rhs.redirects.begin(); redit != rhs.redirects.end(); ++redit) {
      reverse_redirects.insert(pair<size_t, size_t>(redit->second, redit->first));
    }
    map<size_t, size_t> new_redirects;
    multimap<size_t, size_t>::const_iterator revit = reverse_redirects.begin();
    for (; revit != reverse_redirects.end(); ++revit) {
      if (new_redirects.find(revit->second) == new_redirects.end()) {
        new_redirects.insert(pair<size_t, size_t>(revit->second, revit->first));
      }
    }
    p.redirects.swap(new_redirects);
    for (redit = p.redirects.begin(); redit != p.redirects.end(); ++redit) {
      map<size_t, Restriction>::iterator restit = p.restrictions.find(redit->first);
      if (restit != p.restrictions.end()) {
        Restriction &r = p.lookup(redit->second);
        r = r.intersectWith(restit->second);
        if (r.empty()) {
          p.forced_empty = true;
          return p;
        }
        p.restrictions.erase(restit);
      }
    }
    return p;
  }
  Pattern minus(const Pattern &rhs, set<Pattern> &disjuncts) const {
    if (this->empty()) {
      return *this;
    }
    if (rhs.empty()) {
      disjuncts.insert(*this);
      return rhs;
    }
    size_t arity = this->atom.getArity();
    if (this->atom.getType() != rhs.atom.getType() || arity != rhs.atom.getArity()) {
      disjuncts.insert(*this);
      Pattern p(this->atom);
      p.forced_empty = true;
      return p;
    }
    if (this->atom.getType() == ATOM &&
        !this->atom.getPred().equals(rhs.atom.getPred())) {
      disjuncts.insert(*this);
      Pattern p(this->atom);
      p.forced_empty = true;
      return p;
    }
    Pattern inter = this->intersectWith(rhs);
    if (inter.empty()) {
      disjuncts.insert(*this);
      return inter;
    }
#if !PARTITION_RISKY
    size_t i;
    for (i = 1; i <= arity; ++i) {
      Pattern p(*this);
      Restriction &r = p.lookup(i);
      r = r.minus(inter.lookupc(i));
      if (!p.empty()) {
        if (!p.redirects.empty() || !rhs.redirects.empty()) {
          cerr << "[ERROR] Cannot handle Pattern difference when redirects are involved.  Minus: " << *this << "     " << rhs << endl;
          THROW(TraceableException, "Cannot handle Pattern difference when redirects are involved.");
        }
        disjuncts.insert(p);
      }
    }
#else
#warning Setting PARTITION_RISKY means that the minus operation on patterns will be exponential instead of linear time, where N is the arity of the patterns.
    size_t i;
    size_t max = (1 << arity) - 1;
    for (i = 0; i < max; ++i) {
      Pattern p(*this);
      size_t j;
      for (j = 1; !p.empty() && j <= arity; ++j) {
        Restriction &r = p.lookup(j);
        if (((1 << (j-1)) & i) == 0) {
          r = r.minus(inter.lookupc(j));
        } else {
          r = inter.lookupc(j);
        }
      }
      if (!p.empty()) {
        if (!p.redirects.empty() || !rhs.redirects.empty()) {
          cerr << "[ERROR] Cannot handle Pattern difference when redirects are involved.  Minus: " << *this << "     " << rhs << endl;
          THROW(TraceableException, "Cannot handle Pattern difference when redirects are involved.");
        }
        disjuncts.insert(p);
      }
    }
#endif
    return inter;
  }
  void minus(const set<Pattern> &rights, set<Pattern> &disjuncts, set<Pattern> *intersections) const {
    set<Pattern> current;
    current.insert(*this);
    deque<Pattern> inters;
    set<Pattern>::const_iterator right = rights.begin();
    for (; right != rights.end(); ++right) {
      set<Pattern> next;
      set<Pattern>::const_iterator cur = current.begin();
      for (; cur != current.end(); ++cur) {
        Pattern inter = cur->minus(*right, next);
        if (intersections != NULL) {
          if (inters.empty()) {
            inters.push_back(inter);
          } else if (!inter.empty()) {
            Pattern oldinter = inters.back();
            inters.pop_back();
            if (oldinter.empty()) {
              inters.push_back(inter);
            } else {
              set<Pattern> diffinter;
              inter = inter.minus(oldinter, diffinter);
              inters.insert(inters.end(), diffinter.begin(), diffinter.end());
              inters.push_back(inter);
            }
          }
        }
      }
      current.swap(next);
    }
    disjuncts.insert(current.begin(), current.end());
    if (intersections != NULL) {
      if (inters.back().empty()) {
        inters.pop_back();
      }
      intersections->insert(inters.begin(), inters.end());
    }
  }
  size_t partitionWith(const Pattern &rhs, set<Pattern> &partitions) const {
    size_t old_size = partitions.size();
    this->minus(rhs, partitions);
    Pattern inter = rhs.minus(*this, partitions);
    if (!inter.empty()) {
      partitions.insert(inter);
    }
    return partitions.size() - old_size;
  }
#if !PARTITION_RISKY
#warning Warning: calling partition when PARTITION_RISKY is not set doesn't exactly partition the patterns.
#endif
  static bool partition(set<Pattern> &patterns) {
    if (patterns.size() <= 1) {
      return false;
    }
    set<Pattern>::iterator patt = patterns.begin();
    set<Pattern> partitions;
    partitions.insert(*patt);
    size_t count = 1;
    for (++patt; patt != patterns.end(); ++patt) {
      cerr << "[FINE] " << ++count << '/' << patterns.size() << " Partitioning " << *patt << endl;
      set<Pattern> newparts;
      set<Pattern>::const_iterator partit = partitions.begin();
      for (; partit != partitions.end(); ++partit) {
        if (partit->isGround()) {
          newparts.insert(*partit);
        } else {
          Pattern inter = partit->minus(*patt, newparts);
          if (!inter.empty()) {
            newparts.insert(inter);
          }
        }
      }
      if (patt->isGround()) {
        newparts.insert(*patt);
      } else {
        set<Pattern> lpatts;
        lpatts.insert(*patt);
        for (partit = partitions.begin(); partit != partitions.end(); ++partit) {
          set<Pattern> rpatts;
          set<Pattern>::const_iterator lpit = lpatts.begin();
          for (; lpit != lpatts.end(); ++lpit) {
            if (lpit->isGround()) {
              // if lpit is ground, no need to keep iterating over it
              // in an attempt to further partition in.
              newparts.insert(*lpit);
            } else {
              lpit->minus(*partit, rpatts);
            }
          }
          lpatts.swap(rpatts);
        }
        newparts.insert(lpatts.begin(), lpatts.end());
      }
      partitions.swap(newparts);
    }
    bool changed = patterns.size() != partitions.size() ||
                   !equal(patterns.begin(), patterns.end(), partitions.begin());
    patterns.swap(partitions);
    return changed;
  }
  bool operator<(const Pattern &rhs) const {
    if (rhs.empty()) {
      return false;
    }
    if (this->empty()) {
      return true;
    }
    if (this->atom.getType() != rhs.atom.getType()) {
      return this->atom.getType() < rhs.atom.getType();
    }
    if (this->atom.getArity() != rhs.atom.getArity()) {
      return this->atom.getArity() < rhs.atom.getArity();
    }
    if (this->atom.getType() == ATOM || this->atom.getType() == EXTERNAL) {
      int c = RIFConst::cmp(this->atom.getPred(), rhs.atom.getPred());
      if (c != 0) {
        return c < 0;
      }
    }
    size_t i;
    for (i = 1; i <= this->atom.getArity(); ++i) {
      const Restriction &r1 = this->lookupc(i);
      const Restriction &r2 = rhs.lookupc(i);
      if (r1 < r2) {
        return true;
      }
      if (r2 < r1) {
        return false;
      }
    }
    return false;
  }
  bool operator<=(const Pattern &rhs) const {
    return !(rhs < *this);
  }
  bool operator==(const Pattern &rhs) const {
    return !(rhs < *this) && !(*this < rhs);
  }
  bool operator>=(const Pattern &rhs) const {
    return !(*this < rhs);
  }
  bool operator>(const Pattern &rhs) const {
    return rhs < *this;
  }
  bool operator!=(const Pattern &rhs) const {
    return rhs < *this || *this < rhs;
  }
  bool equivalentTo(const Pattern &rhs) const {
    return *this == rhs;
  }
  bool subsetOf(const Pattern &rhs) const {
    return this->intersectWith(rhs) == *this;
  }
  bool properSubsetOf(const Pattern &rhs) const {
    return this->intersectWith(rhs) == *this &&
           rhs.intersectWith(*this) != rhs;
  }
  bool overlapsWith(const Pattern &rhs) const {
    Pattern p = this->intersectWith(rhs);
    return !p.empty();
  }
  bool contains(const RIFAtomic &a) {
    if (!a.isGround()) {
      THROW(TraceableException, "Illegal argument: a must be ground.");
    }
    if (this->atom.getType() != a.getType() ||
        this->atom.getArity() != a.getArity()) {
      return false;
    }
    if (this->atom.getType() == ATOM &&
        !this->atom.getPred().equals(a.getPred())) {
      return false;
    }
    size_t i;
    for (i = 1; i <= this->atom.getArity(); ++i) {
      if (!this->lookupc(i).contains(a.getPart(i))) {
        return false;
      }
    }
    return true;
  }
  void swap(Pattern &p) {
    std::swap(this->forced_empty, p.forced_empty);
    std::swap(this->atom, p.atom);
    this->restrictions.swap(p.restrictions);
    this->redirects.swap(p.redirects);
    this->index.swap(p.index);
  }
  Pattern &operator=(const Pattern &rhs) {
    this->forced_empty = rhs.forced_empty;
    this->atom = rhs.atom;
    map<size_t, Restriction> rest(rhs.restrictions);
    this->restrictions.swap(rest);
    map<size_t, size_t> redir(rhs.redirects);
    this->redirects.swap(redir);
    RMap idx(rhs.index);
    this->index.swap(idx);
    return *this;
  }
};

#define ABBREV 1

ostream &operator<<(ostream &stream, const Pattern &pattern) {
  if (pattern.empty()) {
    return stream << "Or()";
  }
  RIFAtomic atom(pattern.getAtom());
  stream << "And( " << atom;
  RRMap restrictions (RIFTerm::cmplt0);
  pattern.intersectRestrictions(restrictions);
  RRMap::const_iterator it = restrictions.begin();
  for (; it != restrictions.end(); ++it) {
    if (it->first.isGround() || it->second.full()) {
      continue;
    }
    if (it->second.disallows()) {
      stream << " Not(";
    }
#if ABBREV
    stream << " { " << it->first << " : ";
#else
    stream << " External(\"http://www.w3.org/2007/rif-builtin-predicate#list-contains\"^^<http://www.w3.org/2007/rif#iri>(List(";
#endif
    const TSet &terms = it->second.getTerms();
    TSet::const_iterator tit = terms.begin();
    for (; tit != terms.end(); ++tit) {
      stream << ' ' << *tit;
    }
#if ABBREV
    stream << " }";
#else
    stream << " ) " << it->first << "))";
#endif
    if (it->second.disallows()) {
      stream << " )";
    }
  }
  return stream << " )";
}

class Rule;
ostream &operator<<(ostream &stream, const Rule &rule);

class Rule {
private:
  multiset<Pattern> pconds;
  multiset<Pattern> nconds;
  multiset<Pattern> actions;
  bool assertive;
  void intersectRestrictions(const multiset<Pattern> &patts, RRMap &restrictions) const {
    multiset<Pattern>::const_iterator it = patts.begin();
    for (; it != patts.end(); ++it) {
      it->intersectRestrictions(restrictions);
    }
  }
  void restrictPatternSet(multiset<Pattern> &patts, const RRMap &restrictions) {
    multiset<Pattern> newpatts;
    multiset<Pattern>::const_iterator it = patts.begin();
    for (; it != patts.end(); ++it) {
      Pattern newp(*it);
      newp.restrictTerms(restrictions);
      newpatts.insert(newp);
    }
    patts.swap(newpatts);
  }
  void propogateRestrictions() {
    RRMap newrestrictions (RIFTerm::cmplt0);
    this->getRestrictions(newrestrictions);
    this->forceRestrictions(newrestrictions);
  }
  void joincode(vector<size_t> &code) const {
    size_t i, j;
    i = j = 0;
    vector<size_t> joins;
    typedef set<RIFAtomic,
                bool(*)(const RIFAtomic &, const RIFAtomic &)>
            AtomSet;
    AtomSet atoms (RIFAtomic::cmplt0);
    multiset<Pattern>::const_iterator it = this->pconds.begin();
    for (; it != this->pconds.end(); ++it) {
      atoms.insert(it->getAtom());
    }
    for (it = this->nconds.begin(); it != this->nconds.end(); ++it) {
      atoms.insert(it->getAtom());
    }
    for (it = this->actions.begin(); it != this->actions.end(); ++it) {
      atoms.insert(it->getAtom());
    }
    AtomSet::const_iterator it1 = atoms.begin();
    for (; it1 != atoms.end(); ++it1) {
      AtomSet::const_iterator it2 = it1;
      for (++it2; it2 != atoms.end(); ++it2) {
        RMap index (RIFTerm::cmplt0);
        size_t k;
        for (k = 1; k <= it2->getArity(); ++k) {
          index[it2->getPart(k)] = k;
        }
        for (k = 1; k <= it1->getArity(); ++k) {
          RIFTerm term = it1->getPart(k);
          if (term.isGround()) {
            continue;
          }
          RMap::const_iterator rit = index.find(term);
          if (rit != index.end()) {
            joins.push_back(i);
            joins.push_back(k);
            joins.push_back(j);
            joins.push_back(rit->second);
          }
        }
        ++j;
      }
      ++i;
    }
    code.swap(joins);

#if 0
    cerr << "[JOIN]";
    for (i = 0; i < code.size(); ++i) {
      cerr << ' '<< code[i];
    }
    cerr << endl;
#endif
  }
public:
  Rule()
      : assertive(true) {
    // do nothing
  }
  Rule(const bool assertive)
      : assertive(assertive) {
    // do nothing
  }
  Rule(const Rule &copy)
      : pconds(copy.pconds), nconds(copy.nconds), actions(copy.actions),
        assertive(copy.assertive) {
    // do nothing
  }
  ~Rule() {
    // do nothing
  }
  bool isAssertive() const {
    return this->assertive;
  }
  void getRestrictions(RRMap &restrictions) const {
    this->intersectRestrictions(this->pconds, restrictions);
    this->intersectRestrictions(this->nconds, restrictions);
    this->intersectRestrictions(this->actions, restrictions);
  }
  void forceRestrictions(const RRMap &restrictions) {
    this->restrictPatternSet(this->pconds, restrictions);
    this->restrictPatternSet(this->nconds, restrictions);
    this->restrictPatternSet(this->actions, restrictions);
  }
  const multiset<Pattern> &getPositiveConditions() const {
    return this->pconds;
  }
  const multiset<Pattern> &getNegativeConditions() const {
    return this->nconds;
  }
  const multiset<Pattern> &getActions() const {
    return this->actions;
  }
  size_t getPatterns(set<Pattern> &patterns) const {
    size_t old_size = patterns.size();
    patterns.insert(this->pconds.begin(), this->pconds.end());
    patterns.insert(this->nconds.begin(), this->nconds.end());
    patterns.insert(this->actions.begin(), this->actions.end());
    return patterns.size() - old_size;
  }
  void addCondition(const bool sign, const Pattern &p) {
    if (sign) {
      this->pconds.insert(p);
    } else {
      this->nconds.insert(p);
    }
    this->propogateRestrictions();
  }
  void removeCondition(const bool sign, multiset<Pattern>::const_iterator it) {
    if (sign) {
      this->pconds.erase(it);
    } else {
      this->nconds.erase(it);
    }
  }
  void removeCondition(const bool sign, const Pattern &p, const bool all) {
    if (sign) {
      if (all) {
        this->pconds.erase(p);
      } else {
        multiset<Pattern>::iterator it = this->pconds.find(p);
        if (it != this->pconds.end()) {
          this->pconds.erase(it);
        }
      }
    } else {
      if (all) {
        this->nconds.erase(p);
      } else {
        multiset<Pattern>::iterator it = this->nconds.find(p);
        if (it != this->nconds.end()) {
          this->nconds.erase(it);
        }
      }
    }
  }
  void addAction(const Pattern &p) {
    this->actions.insert(p);
    this->propogateRestrictions();
  }
  void removeAction(multiset<Pattern>::const_iterator it) {
    this->actions.erase(it);
  }
  void removeAction(const Pattern &p, const bool all) {
    if (all) {
      this->actions.erase(p);
    } else {
      multiset<Pattern>::iterator it = this->actions.find(p);
      if (it != this->actions.end()) {
        this->actions.erase(it);
      }
    }
  }
  bool tautology() const {
    multiset<Pattern>::const_iterator it = this->actions.begin();
    for (; it != this->actions.end(); ++it) {
      multiset<Pattern>::const_iterator it2 =
          this->assertive ? this->pconds.find(*it) : this->nconds.find(*it);
      if (it2 == (this->assertive ? this->pconds.end() : this->nconds.end())) {
        return false;
      }
      RIFAtomic hatom = it->getAtom();
      RIFAtomic catom = it2->getAtom();
      size_t i;
      for (i = 1; i <= hatom.getArity(); ++i) {
        RIFTerm hterm = hatom.getPart(i);
        RIFTerm cterm = catom.getPart(i);
        if (!hterm.isGround() && !cterm.isGround() && !hterm.equals(cterm)) {
          return false;
        }
      }
    }
    return true;
  }
  bool empty() const {
    multiset<Pattern>::const_iterator it = this->pconds.begin();
    for (; it != this->pconds.end(); ++it) {
      if (it->empty()) {
        return true;
      }
    }
    it = this->nconds.begin();
    for (; it != this->nconds.end(); ++it) {
      if (it->empty()) {
        return true;
      }
    }
    it = this->actions.begin();
    for (; it != this->actions.end(); ++it) {
      if (it->empty()) {
        return true;
      }
    }
    return false;
  }
  bool isGround() const {
    if (this->empty()) {
      return true;
    }
    multiset<Pattern>::const_iterator it = this->pconds.begin();
    for (; it != this->pconds.end(); ++it) {
      if (!it->isGround()) {
        return false;
      }
    }
    for (it = this->nconds.begin(); it != this->nconds.end(); ++it) {
      if (!it->isGround()) {
        return false;
      }
    }
    for (it = this->actions.begin(); it != this->actions.end(); ++it) {
      if (!it->isGround()) {
        return false;
      }
    }
    return true;
  }
  void partitionMatches(const set<Pattern> &matchers, set<Rule> &partitioned_rules) const {
    if (this->nconds.empty()) {
      if (this->pconds.size() <= 0) {
        partitioned_rules.insert(*this);
        return;
      }
      Rule r(*this);
      set<Pattern>::const_iterator pit = r.pconds.begin();
      set<Pattern> partitions;
      partitions.insert(*pit);
      set<Pattern>::const_iterator mit = matchers.begin();
      for (; mit != matchers.end(); ++mit) {
        Pattern inter = pit->intersectWith(*mit);
        if (!inter.empty()) {
          partitions.insert(inter);
        }
      }
      Pattern::partition(partitions);
      r.pconds.erase(pit);
      set<Pattern>::const_iterator partition = partitions.begin();
      for (; partition != partitions.end(); ++partition) {
        Rule r2(r);
        r2.addCondition(true, *partition);
        if (!r2.empty()) {
          partitioned_rules.insert(r2);
        }
      }
      return;
    }
    cerr << "[ERROR] Got lazy... don't care about and haven't supported negation here." << endl;
    THROW(TraceableException, "[ERROR] Got lazy... don't care about and haven't supported negation here.");
  }
  bool hasGroundActions() const {
    multiset<Pattern>::const_iterator it = this->actions.begin();
    for (; it != this->actions.end(); ++it) {
      if (!it->isGround()) {
        return false;
      }
    }
    return true;
  }
  void split(const Pattern &splitter, set<Rule> &unmatched, set<Rule> &matched) const {
    if (this->actions.size() > 1) {
      cerr << "[ERROR] Splitting of rules with multiple actions is currently unsupported." << endl;
      THROW(TraceableException, "Splitting of rules with multiple actions is currently unsupported.");
    }
    if (this->actions.size() <= 0) {
      unmatched.insert(*this);
      return;
    }
    multiset<Pattern>::const_iterator action = this->actions.begin();
    if (action->isGround()) {
      if (action->subsetOf(splitter)) {
        matched.insert(*this);
      } else {
        unmatched.insert(*this);
      }
    }
    set<Pattern> nomatch;
    Pattern match = action->minus(splitter, nomatch);
    if (match.empty()) {
      unmatched.insert(*this);
      return;
    } else {
      Rule r(*this);
      r.actions.clear();
      r.addAction(match);
      matched.insert(r);
    }
    set<Pattern>::const_iterator it = nomatch.begin();
    for (; it != nomatch.end(); ++it) {
      Rule r(*this);
      r.actions.clear();
      r.addAction(*it);
      unmatched.insert(r);
    }
  }
  void split(const set<Pattern> &splitters, set<Rule> &unmatched, set<Rule> &matched) const {
    set<Rule> rules;
    rules.insert(*this);
    set<Pattern>::const_iterator splitter = splitters.begin();
    for (; splitter != splitters.end(); ++splitter) {
      set<Rule> nomatch;
      set<Rule>::const_iterator it = rules.begin();
      for (; it != rules.end(); ++it) {
        it->split(*splitter, nomatch, matched);
      }
      rules.swap(nomatch);
    }
    unmatched.insert(rules.begin(), rules.end());
  }
  bool onlyMatches(const set<Pattern> &patterns, const size_t except) const {
    size_t exc = 0;
    set<Pattern> conds;
    conds.insert(this->pconds.begin(), this->pconds.end());
    conds.insert(this->nconds.begin(), this->nconds.end());
    set<Pattern>::const_iterator cit = conds.begin();
    for (; cit != conds.end(); ++cit) {
      set<Pattern> current;
      current.insert(*cit);
      set<Pattern>::const_iterator pit = patterns.begin();
      for (; !current.empty() && pit != patterns.end(); ++pit) {
        set<Pattern> next;
        set<Pattern>::const_iterator curit = current.begin();
        for (; curit != current.end(); ++curit) {
          curit->minus(*pit, next);
        }
        current.swap(next);
      }
      if (!current.empty() && ++exc > except) {
        return false;
      }
    }
    return true;
  }
  bool onlyMatches(const set<Pattern> &patterns) const {
    return this->onlyMatches(patterns, 0);
  }
  bool onlyMatches(const Pattern &pattern, const size_t except) const {
    set<Pattern> patterns;
    patterns.insert(pattern);
    return this->onlyMatches(patterns, except);
  }
  bool onlyMatches(const Pattern &pattern) const {
    return this->onlyMatches(pattern, 0);
  }
  bool couldImpact(const Pattern &pattern) const {
    set<Pattern>::const_iterator it = this->actions.begin();
    for (; it != this->actions.end(); ++it) {
      if (pattern.overlapsWith(*it)) {
        return true;
      }
    }
    return false;
  }
  bool couldImpact(const set<Pattern> &patterns) const {
    set<Pattern>::const_iterator it;
    for (it = patterns.begin(); it != patterns.end(); ++it) {
      if (this->couldImpact(*it)) {
        return true;
      }
    }
    return false;
  }
  bool operator<(const Rule &rhs) const {
    if (rhs.empty()) {
      return false;
    }
    if (this->empty()) {
      return true;
    }
    if (this->assertive != rhs.assertive) {
      return rhs.assertive;
    }
    if (this->pconds.size() != rhs.pconds.size()) {
      return this->pconds.size() < rhs.pconds.size();
    }
    if (this->nconds.size() != rhs.nconds.size()) {
      return this->nconds.size() < rhs.nconds.size();
    }
    if (this->actions.size() != rhs.actions.size()) {
      return this->actions.size() < rhs.actions.size();
    }
    multiset<Pattern>::const_iterator it1 = this->pconds.begin();
    multiset<Pattern>::const_iterator it2 = rhs.pconds.begin();
    multiset<Pattern>::const_iterator end = this->pconds.end();
    for (; it1 != end; ++it1) {
      if (*it1 < *it2) {
        return true;
      }
      if (*it2 < *it1) {
        return false;
      }
      ++it2;
    }
    it1 = this->nconds.begin();
    it2 = rhs.nconds.begin();
    end = this->nconds.end();
    for (; it1 != end; ++it1) {
      if (*it1 < *it2) {
        return true;
      }
      if (*it2 < *it1) {
        return false;
      }
      ++it2;
    }
    it1 = this->actions.begin();
    it2 = rhs.actions.begin();
    end = this->actions.end();
    for (; it1 != end; ++it1) {
      if (*it1 < *it2) {
        return true;
      }
      if (*it2 < *it1) {
        return false;
      }
      ++it2;
    }
    vector<size_t> code1, code2;
    this->joincode(code1);
    rhs.joincode(code2);
    if (code1.size() != code2.size()) {
      return code1.size() < code2.size();
    }
    vector<size_t>::const_iterator cit1 = code1.begin();
    vector<size_t>::const_iterator cit2 = code2.begin();
    for (; cit1 != code1.end(); ++cit1) {
      if (*cit1 != *cit2) {
        return *cit1 < *cit2;
      }
      ++cit2;
    }
    return false;
  }
  bool operator<=(const Rule &rhs) const {
    return !(rhs < *this);
  }
  bool operator==(const Rule &rhs) const {
    return !(rhs < *this) && !(*this < rhs);
  }
  bool operator>=(const Rule &rhs) const {
    return !(*this < rhs);
  }
  bool operator>(const Rule &rhs) const {
    return rhs < *this;
  }
  bool operator!=(const Rule &rhs) const {
    return rhs < *this || *this < rhs;
  }
  void swap(Rule &r) {
    std::swap(this->assertive, r.assertive);
    this->pconds.swap(r.pconds);
    this->nconds.swap(r.nconds);
    this->actions.swap(r.actions);
  }
  Rule &operator=(const Rule &rhs) {
    this->assertive = rhs.assertive;
    multiset<Pattern> newpconds(rhs.pconds);
    multiset<Pattern> newnconds(rhs.nconds);
    multiset<Pattern> newactions(rhs.actions);
    this->pconds.swap(newpconds);
    this->nconds.swap(newnconds);
    this->actions.swap(newactions);
    return *this;
  }
};

ostream &operator<<(ostream &stream, const Rule &rule) {
  if (rule.empty()) {
    return stream << "If Or() Then Do()" << endl;
  }
  stream << "If And(";
  const multiset<Pattern> &pconds = rule.getPositiveConditions();
  multiset<Pattern>::const_iterator it = pconds.begin();
  for (; it != pconds.end(); ++it) {
    RIFAtomic a(it->getAtom());
    stream << ' ' << a;
  }
  const multiset<Pattern> &nconds = rule.getNegativeConditions();
  it = nconds.begin();
  for (; it != nconds.end(); ++it) {
    RIFAtomic a(it->getAtom());
    stream << " Not(" << a << ")";
  }
  RRMap restrictions (RIFTerm::cmplt0);
  rule.getRestrictions(restrictions);
  RRMap::const_iterator rit = restrictions.begin();
  for (; rit != restrictions.end(); ++rit) {
    if (rit->first.isGround() || rit->second.full()) {
      continue;
    }
    stream << ' ';
    if (rit->second.disallows()) {
      stream << "Not(";
    }
#if ABBREV
    stream << " { " << rit->first << " : ";
#else
    stream << "External(\"http://www.w3.org/2007/rif-builtin-predicate#list-contains\"^^<http://www.w3.org/2007/rif#iri>(List(";
#endif
    const TSet &terms = rit->second.getTerms();
    TSet::const_iterator tit = terms.begin();
    for (; tit != terms.end(); ++tit) {
      stream << ' ' << *tit;
    }
#if ABBREV
    stream << " }";
#else
    stream << " ) " << rit->first << "))";
#endif
    if (rit->second.disallows()) {
      stream << ")";
    }
  }
  stream << ") Then Do(";
  const multiset<Pattern> &actions = rule.getActions();
  it = actions.begin();
  for (; it != actions.end(); ++it) {
    if (rule.isAssertive()) {
      RIFAtomic a(it->getAtom());
      stream << " Assert(" << a << ")";
    } else {
      RIFAtomic a(it->getAtom());
      stream << " Retract(" << a << ")";
    }
  }
  stream << ")";
  return stream;
}

DPtr<uint8_t> *cstr2dptr(const char *cstr) {
  try {
    size_t len = strlen(cstr);
    DPtr<uint8_t> *p;
    NEW(p, MPtr<uint8_t>, len);
    ascii_strncpy(p->dptr(), cstr, len);
    return p;
  } RETHROW_BAD_ALLOC
}

void read_rif_rules(deque<RIFRule> &rules, deque<RIFCondition> &repls, deque<RIFCondition> &norepls, deque<RIFCondition> &arbs, deque<RIFCondition> &noarbs) {
  string line;
  while (!cin.eof()) {
    getline(cin, line);
    if (line.size() >= 18 && line.substr(0, 18) == string("#PRAGMA REPLICATE ")) {
      string cond = line.substr(18);
      DPtr<uint8_t> *p = cstr2dptr(cond.c_str());
      RIFCondition c = RIFCondition::parse(p);
      p->drop();
      repls.push_back(c);
      continue;
    } else if (line.size() >= 22 && line.substr(0, 22) == string("#PRAGMA NOT REPLICATE ")) {
      string cond = line.substr(22);
      DPtr<uint8_t> *p = cstr2dptr(cond.c_str());
      RIFCondition c = RIFCondition::parse(p);
      p->drop();
      norepls.push_back(c);
      continue;
    } else if (line.size() >= 18 && line.substr(0, 18) == string("#PRAGMA ARBITRARY ")) {
      string cond = line.substr(18);
      DPtr<uint8_t> *p = cstr2dptr(cond.c_str());
      RIFCondition c = RIFCondition::parse(p);
      p->drop();
      arbs.push_back(c);
      continue;
    } else if (line.size() >= 22 && line.substr(0, 22) == string("#PRAGMA NOT ARBITRARY ")) {
      string cond = line.substr(22);
      DPtr<uint8_t> *p = cstr2dptr(cond.c_str());
      RIFCondition c = RIFCondition::parse(p);
      p->drop();
      noarbs.push_back(c);
      continue;
    } else if (line.size() >= 1 && line[0] == '#') {
      // ignore comment
      continue;
    } else if (line.size() <= 1) {
      break;
    }
    DPtr<uint8_t> *p = cstr2dptr(line.c_str());
    RIFRule rule = RIFRule::parse(p);
    p->drop();
    rules.push_back(rule);
  }
}

bool translate_pred_list_contains(const RIFAtomic &pred_list_contains, const bool negated, RRMap &restrictions) {
  if (pred_list_contains.getType() != EXTERNAL) {
    return false;
  }
  RIFConst pred = pred_list_contains.getPred();
  DPtr<uint8_t> *lexform = pred.getLexForm();
  if (lexform->size() != 58 || ascii_strncmp(lexform->dptr(), "http://www.w3.org/2007/rif-builtin-predicate#list-contains", 58) != 0) {
    lexform->drop();
    return false;
  }
  lexform->drop();
  IRIRef datatype = pred.getDatatype();
  DPtr<uint8_t> *utf8str = datatype.getUTF8String();
  if (utf8str->size() != 30 || ascii_strncmp(utf8str->dptr(), "http://www.w3.org/2007/rif#iri", 30) != 0) {
    utf8str->drop();
    return false;
  }
  utf8str->drop();
  DPtr<RIFTerm> *args = pred_list_contains.getArgs();
  if (args->size() != 2) {
    args->drop();
    cerr << "[ERROR] pred:list-contains should have exactly two arguments, but found " << pred_list_contains << endl;
    THROW(TraceableException, "pred:list-contains should have exactly two arguments.");
  }
  RIFTerm *list = args->dptr();
  if (list->getType() != LIST) {
    args->drop();
    cerr << "[ERROR] The first argument of pred:list-contains should be a list, but found " << *list << endl;
    THROW(TraceableException, "The first argument of pred:list-contains should be a list.");
  }
  Restriction restriction(!negated);
  DPtr<RIFTerm> *items = list->getItems();
  RIFTerm *item = items->dptr();
  RIFTerm *enditems = item + items->size();
  for (; item != enditems; ++item) {
    restriction.addTerm(*item);
  }
  items->drop();

  RIFTerm *bounded = list + 1;
  if (bounded->isGround()) {
    args->drop();
    cerr << "[ERROR] The second argument of pred:list-contains should be a non-ground term, but found " << *bounded << endl;
    THROW(TraceableException, "The second argument of pred:list-contains should be a non-ground term.");
  }
  RRMap::iterator rit = restrictions.find(*bounded);
  if (rit == restrictions.end()) {
    restrictions[*bounded] = restriction;
  } else {
    rit->second = rit->second.intersectWith(restriction);
  }
  args->drop();
  return true;
}

Pattern translate_repl_pragma(const RIFCondition &repl_pragma) {
  if (repl_pragma.getType() != CONJUNCTION) {
    cerr << "[ERROR] Replication pragmas must be conjunction formulas, but found " << repl_pragma << endl;
    THROW(TraceableException, "Replication pragmas must be conjunction formulas.");
  }
  DPtr<RIFCondition> *subformulas = repl_pragma.getSubformulas();
  if (subformulas->size() < 1) {
    subformulas->drop();
    cerr << "[ERROR] Replication pragma must not be And()." << endl;
    THROW(TraceableException, "Replication pragmas must not be And().");
  }
  RIFCondition *subformula = subformulas->dptr();
  RIFCondition *endsubf = subformula + subformulas->size();
  if (subformula->getType() != ATOMIC) {
    subformulas->drop();
    cerr << "[ERROR] First subformula of replication pragma must be atomic, but found " << *subformula << endl;
    THROW(TraceableException, "First subformula of replication pragma must be atomic.");
  }
  RIFAtomic atom = subformula->getAtomic();
  if (atom.getType() == EXTERNAL) {
    subformulas->drop();
    cerr << "[ERROR] First atom must not be a builtin, but found " << *subformula << endl;
    THROW(TraceableException, "First atom must not be a builtin.");
  }
  Pattern pattern(atom);
  RRMap restrictions (RIFTerm::cmplt0);
  for (++subformula; subformula != endsubf; ++subformula) {
    bool negated = subformula->getType() == NEGATION;
    if (!negated && subformula->getType() != ATOMIC) {
      subformulas->drop();
      cerr << "[ERROR] Subformulas of replication pragma must be atomic or negation of atomic, but found " << *subformula << endl;
      THROW(TraceableException, "Subformulas of replication pragma must be atomic or negation of atomic.");
    }
    if (negated) {
      RIFCondition subsubformula = subformula->getSubformula();
      if (subsubformula.getType() != ATOMIC) {
        subformulas->drop();
        cerr << "[ERROR] Subformulas of replication pragma must be atomic or negation of atomic, but found " << *subformula << endl;
        THROW(TraceableException, "Subformulas of replication pragma must be atomic or negation of atomic.");
      }
      atom = subsubformula.getAtomic();
    } else {
      atom = subformula->getAtomic();
    }
    if (!translate_pred_list_contains(atom, negated, restrictions)) {
      subformulas->drop();
      cerr << "[ERROR] All but first subformula in replication pragma must be (possibly negated) pred:list-contains, but found " << atom << endl;
      THROW(TraceableException, "All but first subformula in replication pragma must be (possibly negated) pred:list-contains.");
    }
  }
  subformulas->drop();
  pattern.restrictTerms(restrictions);
  return pattern;
}

void translate_repl_pragmas(const deque<RIFCondition> &repl_pragmas, set<Pattern> &patterns) {
  deque<RIFCondition>::const_iterator it;
  for (it = repl_pragmas.begin(); it != repl_pragmas.end(); ++it) {
    patterns.insert(translate_repl_pragma(*it));
  }
}

Rule translate_rif_rule(const RIFRule &rif_rule) {
  RIFActionBlock action_block = rif_rule.getActionBlock();
  DPtr<RIFAction> *actions = action_block.getActions();
  const RIFAction *action = actions->dptr();
  const RIFAction *end = action + actions->size();
  bool assertive = (action == end || action->getType() == ASSERT_FACT);
  Rule rule(assertive);
  for (; action != end; ++action) {
    if (assertive) {
      if (action->getType() != ASSERT_FACT) {
        actions->drop();
        cerr << "[ERROR] Expected all assertion actions but found " << *action << endl;
        THROW(TraceableException, "Expected all assertion actions.");
      }
    } else {
      if (action->getType() != RETRACT_FACT) {
        actions->drop();
        cerr << "[ERROR] Expected all /fact/ retraction actions but found " << *action << endl;
        THROW(TraceableException, "Expected all /fact/ retraction actions.");
      }
    }
    RIFAtomic a(action->getTargetAtomic());
    Pattern pattern(a);
    rule.addAction(pattern);
  }
  actions->drop();
  RRMap restrictions (RIFTerm::cmplt0);
  RIFCondition condition = rif_rule.getCondition();
  if (condition.getType() != CONJUNCTION && condition.getType() != ATOMIC) {
    cerr << "[ERROR] Body of rule must be conjunction or atomic, but found " << condition << endl;
    THROW(TraceableException, "Body of rule must be conjunction or atomic.");
  }
  if (condition.getType() == ATOMIC) {
    RIFAtomic atom(condition.getAtomic());
    Pattern pattern(atom);
    rule.addCondition(true, atom);
    return rule;
  }
  DPtr<RIFCondition> *subformulas = condition.getSubformulas();
  RIFCondition *subformula = subformulas->dptr();
  RIFCondition *end2 = subformula + subformulas->size();
  for (; subformula != end2; ++subformula) {
    if (subformula->getType() != NEGATION && subformula->getType() != ATOMIC) {
      subformulas->drop();
      cerr << "[ERROR] Subformula must be negation or atomic, but found " << *subformula << endl;
      THROW(TraceableException, "Subformula must be negation or atomic.");
    }
    if (subformula->getType() == ATOMIC) {
      RIFAtomic atom(subformula->getAtomic());
      if (!translate_pred_list_contains(atom, false, restrictions)) {
        Pattern pattern(atom);
        rule.addCondition(true, pattern);
      }
      continue;
    }
    RIFCondition subsubformula = subformula->getSubformula();
    if (subsubformula.getType() != ATOMIC) {
      subformulas->drop();
      cerr << "[ERROR] Negation allowed only on atomic formulas, but found " << *subformula << endl;
      THROW(TraceableException, "Negation allowed only on atomic formulas.");
    }
    RIFAtomic atom(subsubformula.getAtomic());
    if (!translate_pred_list_contains(atom, true, restrictions)) {
      Pattern pattern(atom);
      rule.addCondition(false, pattern);
    }
  }
  subformulas->drop();
  rule.forceRestrictions(restrictions);
  return rule;
}

void translate_rif_rules(const deque<RIFRule> &rif_rules, set<Rule> &rules) {
  deque<RIFRule>::const_iterator it = rif_rules.begin();
  for (; it != rif_rules.end(); ++it) {
    rules.insert(translate_rif_rule(*it));
  }
}

void print_rules(const set<Rule> &rules) {
  set<Rule>::const_iterator it = rules.begin();
  for (; it != rules.end(); ++it) {
    cout << *it << endl;
  }
}

///// SAT STUFF /////

class Var;

map<Var, size_t> var2num;
deque<Var> num2var;
deque<pair<bool, size_t> > cnfdata;
size_t numclauses = 0;

enum Pred {
  REPLICATE = 0,
  ARBITRARY = 1,
  ABANDON   = 2
};

class Var {
private:
  enum Pred pred;
  Pattern *pattern;
  Rule rule;
public:
  Var(const bool replicate, const Pattern &patt)
      : pred(replicate ? REPLICATE : ARBITRARY) {
    NEW(this->pattern, Pattern, patt);
  }
  Var(const Rule &rule)
      : pred(ABANDON), pattern(NULL), rule(rule) {
    // do nothing
  }
  Var(const Var &copy)
      : pred(copy.pred), rule(copy.rule) {
    if (copy.pattern == NULL) {
      this->pattern = NULL;
    } else {
      NEW(this->pattern, Pattern, *copy.pattern);
    }
  }
  ~Var() {
    if (this->pattern != NULL) {
      DELETE(this->pattern);
    }
  }
  enum Pred getPred() const {
    return this->pred;
  }
  const Pattern &getPattern() const {
    return *this->pattern;
  }
  const Rule &getRule() const {
    return this->rule;
  }
  bool operator<(const Var &rhs) const {
    if (this == &rhs) {
      return false;
    }
    if (this->pred != rhs.pred) {
      return this->pred < rhs.pred;
    }
    if (this->pred == ABANDON) {
      return this->rule < rhs.rule;
    }
    return *this->pattern < *rhs.pattern;
  }
  Var &operator=(const Var &rhs) {
    this->pred = rhs.pred;
    this->rule = rhs.rule;
    if (this->pattern != NULL) {
      DELETE(this->pattern);
    }
    if (rhs.pattern == NULL) {
      this->pattern = NULL;
    } else {
      NEW(this->pattern, Pattern, *rhs.pattern);
    }
    return *this;
  }
};

ostream &operator<<(ostream &stream, const Var &var) {
  switch (var.getPred()) {
    case REPLICATE: stream << "REPLICATE"; break;
    case ARBITRARY: stream << "ARBITRARY"; break;
    case ABANDON:   stream << "ABANDON  "; break;
    default: cerr << "[ERROR] Unhandled case: " << var.getPred() << endl;
             THROW(TraceableException, "Unhandled case.");
  }
  stream << ' ';
  if (var.getPred() == ABANDON) {
    stream << var.getRule();
  } else {
    stream << var.getPattern();
  }
  return stream;
}

void make_literal(const bool sign, const Var &var) {
  map<Var, size_t>::const_iterator it = var2num.find(var);
  if (it == var2num.end()) {
    it = var2num.insert(pair<Var, size_t>(var, num2var.size())).first;
    num2var.push_back(var);
  }
  cnfdata.push_back(pair<bool, size_t>(sign, it->second + 1));
}

void make_literal(const bool sign, const bool repl, const Pattern &patt) {
  Var var (repl, patt);
  make_literal(sign, var);
}

void make_literal(const bool sign, const Rule &rule) {
  Var var(rule);
  make_literal(sign, var);
}

void terminate_clause() {
  cnfdata.push_back(pair<bool, size_t>(true, 0));
  ++numclauses;
}

// Simplifies the CNF data, returning false if determined that it is unsatisfiable.
// Returning true makes no determination.
bool simplify_cnfdata() {
#if 0 // needs work, but maybe not necessary?
  typedef pair<multimap<size_t, size_t>::const_iterator, multimap<size_t, size_t>::const_iterator> const_eqrng_t;
  multimap<size_t, size_t> pos_clause_index;
  multimap<size_t, size_t> neg_clause_index;
  size_t i;
  for (i = 0; i < cnfdata.size(); ++i) {
    size_t n = cnfdata[i].second;
    if (n != 0) {
      index.insert(pair<size_t, size_t>(n, i));
    }
  }
#endif
  return true;
}

#define OLDWAY 0

void print_cnf(const set<Rule> &saferules, const set<Rule> &riskyrules, const set<Pattern> &repls, const set<Pattern> &norepls, const set<Pattern> &arbs, const set<Pattern> &noarbs) {
  set<Pattern> patterns;
#if OLDWAY
  patterns.insert(repls.begin(), norepls.end());
  patterns.insert(norepls.begin(), norepls.end());
  patterns.insert(arbs.begin(), arbs.end());
  patterns.insert(noarbs.begin(), noarbs.end());
#endif
  set<Rule> rules(saferules);
#if !TRASH_RISKY
  rules.insert(riskyrules.begin(), riskyrules.end());
#endif
  set<Rule>::const_iterator it;
  for (it = saferules.begin(); it != saferules.end(); ++it) {
#if KEEP_SAFE
    // -X(rule) since it is already known to be safe.
    // This will help the SAT solver to be more efficient.
    make_literal(false, *it);
    terminate_clause();
#endif
    cout << "c SAFE " << *it << endl;
  }
  for (it = riskyrules.begin(); it != riskyrules.end(); ++it) {
#if KEEP_RISKY
    make_literal(false, *it);
    terminate_clause();
#endif
#if TRASH_RISKY
    make_literal(true, *it);
    terminate_clause();
#endif
    cout << "c RISK " << *it << endl;
  }
  for (it = rules.begin(); it != rules.end(); ++it) {
    const multiset<Pattern> &pconds = it->getPositiveConditions();
    const multiset<Pattern> &nconds = it->getNegativeConditions();
    const multiset<Pattern> &actions = it->getActions();
    patterns.insert(pconds.begin(), pconds.end());
    patterns.insert(nconds.begin(), nconds.end());
    patterns.insert(actions.begin(), actions.end());
    multiset<Pattern>::const_iterator pit1, pit2;
    for (pit1 = nconds.begin(); pit1 != nconds.end(); ++pit1) {
      // R(ncond) because it is negated.
      make_literal(true, true, *pit1);
      terminate_clause();
    }
    if (!it->isAssertive()) {
      for (pit1 = actions.begin(); pit1 != actions.end(); ++pit1) {
        // R(action) because it is retracted
        make_literal(true, true, *pit1);
        terminate_clause();
      }
      for (pit1 = pconds.begin(); pit1 != pconds.end(); ++pit1) {
        // R(pcond) because SAT will require it anyway since R(action).
        // This may help the SAT solver be more efficient.
        make_literal(true, true, *pit1);
        terminate_clause();
      }
    } else {
      for (pit1 = actions.begin(); pit1 != actions.end(); ++pit1) {
        for (pit2 = pconds.begin(); pit2 != pconds.end(); ++pit2) {
          // X(rule) or A(action) or R(pcond)
          make_literal(true, *it);
          make_literal(true, false, *pit1);
          make_literal(true, true, *pit2);
          terminate_clause();
        }
        // Hopefully this clause, while not necessary, will improve performance.
        // It says, don't sacrifice the rule if you don't have to.
        // A(action) -> -X(rule)
        // -A(action) or -x(rule)
        make_literal(false, false, *pit1);
        make_literal(false, *it);
        terminate_clause();
      }
      // Hopefully this clause, while not necessary, will improve performance.
      // It says, don't sacrifice the rule if you don't have to.
      // [AND_i R(pcond)] -> -X(rule)
      // [OR_i -R(pcond)] or -X(rule)
      for (pit1 = pconds.begin(); pit1 != pconds.end(); ++pit1) {
        make_literal(false, true, *pit1);
      }
      make_literal(false, *it);
      terminate_clause();
      for (pit1 = pconds.begin(); pit1 != pconds.end(); ++pit1) {
        pit2 = pit1;
        for (++pit2; pit2 != pconds.end(); ++pit2) {
          // -R(pcond1) -> R(pcond2)
          // R(pconds1) or R(pcond2)
          make_literal(true, true, *pit1);
          make_literal(true, true, *pit2);
          terminate_clause();
        }
      }
    }
  }
  set<Pattern>::const_iterator pit1, pit2;
#if OLDWAY
  for (pit1 = repls.begin(); pit1 != repls.end(); ++pit1) {
    // R(repl)
    make_literal(true, true, *pit1);
    terminate_clause();
  }
  for (pit1 = norepls.begin(); pit1 != norepls.end(); ++pit1) {
    // -R(norepl)
    make_literal(false, true, *pit1);
    terminate_clause();
  }
  for (pit1 = arbs.begin(); pit1 != arbs.end(); ++pit1) {
    // A(arb)
    make_literal(true, false, *pit1);
    terminate_clause();
  }
  for (pit1 = noarbs.begin(); pit1 != noarbs.end(); ++pit1) {
    // -A(noarb)
    make_literal(false, false, *pit1);
    terminate_clause();
  }
#if PARTITION_PATTERNS
  set<Pattern> partitions(patterns);
  Pattern::partition(partitions);
  patterns.insert(partitions.begin(), partitions.end());
  for (pit1 = partitions.begin(); pit1 != partitions.end(); ++pit1) {
    cout << "c PART " << *pit1 << endl;
  }
#endif
#endif
  for (pit1 = patterns.begin(); pit1 != patterns.end(); ++pit1) {
    // -R(pattern) or -A(pattern)
    make_literal(false, true, *pit1);
    make_literal(false, false, *pit1);
    terminate_clause();
#if !OLDWAY
    for (pit2 = repls.begin(); pit2 != repls.end(); ++pit2) {
      if (pit1->subsetOf(*pit2)) {
        make_literal(true, true, *pit1);
        terminate_clause();
      } else if (pit1->overlapsWith(*pit2)) {
        make_literal(false, false, *pit1);
        terminate_clause();
      }
    }
    for (pit2 = norepls.begin(); pit2 != norepls.end(); ++pit2) {
      if (pit2->subsetOf(*pit1)) {
        make_literal(false, true, *pit1);
        terminate_clause();
      }
    }
    for (pit2 = arbs.begin(); pit2 != arbs.end(); ++pit2) {
      if (pit1->subsetOf(*pit2)) {
        make_literal(true, false, *pit1);
        terminate_clause();
      } else if (pit1->overlapsWith(*pit2)) {
        make_literal(false, true, *pit1);
        terminate_clause();
      }
    }
    for (pit2 = noarbs.begin(); pit2 != noarbs.end(); ++pit2) {
      if (pit2->subsetOf(*pit1)) {
        make_literal(false, false, *pit1);
        terminate_clause();
      }
    }
#endif
    pit2 = pit1;
    for (++pit2; pit2 != patterns.end(); ++pit2) {
      if (pit1->subsetOf(*pit2)) {
        // For pattern1 <= pattern2
        // A(pattern2) -> A(pattern1)
        // -A(pattern2) or A(pattern1)
        make_literal(false, false, *pit2);
        make_literal(true, false, *pit1);
        terminate_clause();
        // R(pattern2) -> R(pattern1)
        // -R(pattern2) or R(pattern1)
        make_literal(false, true, *pit2);
        make_literal(true, true, *pit1);
        terminate_clause();
      }
      if (pit2->subsetOf(*pit1)) {
        // For pattern2 <= pattern1
        // A(pattern1) -> A(pattern2)
        // -A(pattern1) or A(pattern2)
        make_literal(false, false, *pit1);
        make_literal(true, false, *pit2);
        terminate_clause();
        // R(pattern1) -> R(pattern2)
        // -R(pattern1) or R(pattern2)
        make_literal(false, true, *pit1);
        make_literal(true, true, *pit2);
        terminate_clause();
      }
      if (pit1->overlapsWith(*pit2)) {
        // For pattern1 ^ pattern2 != 0
        // A(pattern1) -> -R(pattern2)
        // -A(pattern1) or -R(pattern2)
        make_literal(false, false, *pit1);
        make_literal(false, true, *pit2);
        terminate_clause();
        // R(pattern1) -> -A(pattern2)
        // -R(pattern1) oro -A(pattern2)
        make_literal(false, true, *pit1);
        make_literal(false, false, *pit2);
        terminate_clause();
      }
    }
  }

  if (!simplify_cnfdata()) {
    cout << "c UNSATISFIABLE" << endl;
    cout << "p cnf 1 2" << endl;
    cout << "1 0" << endl;
    cout << "-1 0" << endl;
    return;
  }

  size_t n;
  for (n = 0; n < num2var.size(); ++n) {
    cout << "c " << (n+1) << ' ' << num2var[n] << endl;
  }
  cout << "p cnf " << num2var.size() << ' ' << numclauses << endl;
  deque<pair<bool, size_t> >::const_iterator cit, prev;
  for (cit = cnfdata.begin(); cit != cnfdata.end(); ++cit) {
    if (cit != cnfdata.begin() && (!prev->first || prev->second != 0)) {
      cout << ' ';
    }
    if (!cit->first) {
      cout << '-';
    }
    cout << cit->second;
    if (cit->first && cit->second == 0) {
      cout << endl;
    }
    prev = cit;
  }
}

void classify_rules(const set<Rule> &rules, const set<Pattern> &forcedrepls, set<Rule> &saferules, set<Rule> &riskyrules,
                                            const set<Pattern> &forcednorepls, const set<Pattern> &forcedarbs, const set<Pattern> &forcednoarbs) {
  cerr << "[INFO] Classify rules." << endl;
#if !KEEP_SAFE
  //#error Having KEEP_SAFE turned off is not yet supported (i.e., no PICK or SLOWPICK).
  set<Rule>::const_iterator rit;
  set<Pattern> partitions(forcedrepls);
  partitions.insert(forcednorepls.begin(), forcednorepls.end());
  partitions.insert(forcedarbs.begin(), forcedarbs.end());
  partitions.insert(forcednoarbs.begin(), forcednoarbs.end());
  for (rit = rules.begin(); rit != rules.end(); ++rit) {
    rit->getPatterns(partitions);
  }
  Pattern::partition(partitions);
  set<Rule> new_rules(rules);
  size_t count, count2;
  for (count = 1;; ++count) {
    cerr << "[INFO] Splitting rules, iteration " << count << endl;
    set<Rule> part_rules;
    count2 = 0;
    for (rit = new_rules.begin(); rit != new_rules.end(); ++rit) {
      cerr << "[FINE] " << ++count2 << '/' << new_rules.size() << " Using " << partitions.size() << " patterns to split " << *rit << endl;
      if (rit->hasGroundActions()) {
        //part_rules.insert(*rit);
        // no need to iterate on this one again; just put in riskyrules
        riskyrules.insert(*rit);
      } else {
        rit->split(partitions, part_rules, part_rules);
      }
    }
    cerr << "[INFO] After splitting, there are " << part_rules.size() << " non-ground rules." << endl;
    new_rules.swap(part_rules);
    if (new_rules.size() == part_rules.size() &&
        equal(new_rules.begin(), new_rules.end(), part_rules.begin())) {
      break;
    }
    set<Pattern> new_partitions;
    for (rit = new_rules.begin(); rit != new_rules.end(); ++rit) {
      rit->getPatterns(new_partitions);
    }
    for (rit = riskyrules.begin(); rit != riskyrules.end(); ++rit) {
      rit->getPatterns(new_partitions);
    }
    cerr << "[INFO] Rules changed, so we need another iteration.  Partitioning " << new_partitions.size() << " new patterns." << endl;
    Pattern::partition(new_partitions);
    partitions.swap(new_partitions);
    cerr << "[INFO] The patterns have been partitioned into " << partitions.size() << " new partitions." << endl;
  }
  riskyrules.insert(new_rules.begin(), new_rules.end());
#else
  set<Rule>::const_iterator rit = rules.begin();
  for (; rit != rules.end(); ++rit) {
    cerr << "[INFO] Is this rule safe or risky? " << *rit << endl;

    cerr << "[INFO] Step zero.  If rule is a tautology, it is safe." << endl;
    if (rit->tautology()) {
      cerr << "[INFO] The rule is a tautology and is therefore safe." << endl;
      saferules.insert(*rit);
      continue;
    }

    // First, check if rule can even produce replicated facts.
    // If not, automatically safe.
    // Second, check if rule body always matches replicated facts.
    // If so, automatically safe.
    cerr << "[INFO] Steps one and two.  Check if it produces replicated facts or can be satisfied by only replicated facts." << endl;
    if (rit->onlyMatches(forcedrepls, 1) && (!rit->couldImpact(forcedrepls) || rit->onlyMatches(forcedrepls))) {
      cerr << "[INFO] Steps one and two determined the rule is safe." << endl;
      saferules.insert(*rit);
      continue;
    }

    // The SAT formulation requires non-assertive rules (those that retract
    // facts) have all possibly retracted facts to be replicated, which means
    // the everything matching the body must also be replicated, or otherwise
    // be risky (not just risky, but bad).
    if (!rit->isAssertive() && !rit->onlyMatches(forcedrepls)) {
      riskyrules.insert(*rit);
      continue;
    }

//    // Third, partition the rule on patterns (expensive operation) and
//    // check each new rule as before.
//    set<Rule> partitioned_rules;
//    partitioned_rules.insert(*rit);
//    partition_rules(partitioned_rules, &forcedrepls);
//    set<Rule>::const_iterator it = partitioned_rules.begin();
//    for (; it != partitioned_rules.end(); ++it) {
//      if (!it->couldImpact(forcedrepls) || it->onlyMatches(forcedrepls)) {
//        saferules.insert(*it);
//      } else {
//        riskyrules.insert(*it);
//      }
//    }

    // Third, split rule actions.  The rule not producing
    // replicated facts is safe.  Continue to "fourth" for the rest.
    cerr << "[INFO] Step three.  Splitting the rule (this may take a while)." << endl;
    set<Rule> match, nomatch;
    rit->split(forcedrepls, nomatch, match);

    // Fourth, restrict rule bodies such that they are instantiated
    // by only replicated facts.  These are safe.  The remaining are risky.
    cerr << "[INFO] Step four.  Restricting the bodies of possibly risky rules." << endl;
    set<Rule>::const_iterator mit;
    for (mit = match.begin(); mit != match.end(); ++mit) {
      set<Rule> partitioned;
      cerr << "[INFO] Step 4a.  Partition matches for risky rule " << *mit << endl;
      mit->partitionMatches(forcedrepls, partitioned);
      set<Rule>::const_iterator partit = partitioned.begin();
      for (; partit != partitioned.end(); ++partit) {
        if (partit->tautology() || partit->onlyMatches(forcedrepls)) {
          cerr << "[INFO] Determined the following to be safe: " << *partit << endl;
          saferules.insert(*partit);
        } else {
          cerr << "[INFO] Determined the following to be risky: " << *partit << endl;
          riskyrules.insert(*partit);
        }
      }
    }

    // Fifth, make sure rules not producing replicated facts are M1.
    cerr << "[INFO] Step five.  Checking for tautology or M1-ness of possibly risky rules." << endl;
    for (mit = nomatch.begin(); mit != nomatch.end(); ++mit) {
      if (mit->tautology() || mit->onlyMatches(forcedrepls, 1)) {
        cerr << "[INFO] Safe: " << *mit << endl;
        saferules.insert(*mit);
      } else {
        cerr << "[INFO] Risky: " << *mit << endl;
        riskyrules.insert(*mit);
      }
    }
  }

//  for (rit = saferules.begin(); rit != saferules.end(); ++rit) {
//    cerr << "[SAFE] " << *rit << endl;
//  }
//  for (rit = riskyrules.begin(); rit != riskyrules.end(); ++rit) {
//    cerr << "[RISK] " << *rit << endl;
//  }
#endif
}

int main(int argc, char **argv) {
  TIME_T(ts_begin, ts_end);
  ts_begin = TIME();
  deque<RIFCondition> repl_pragmas, no_repl_pragmas, arb_pragmas, no_arb_pragmas;
  deque<RIFRule> rif_rules;
  set<Pattern> repl_patterns, no_repl_patterns, arb_patterns, no_arb_patterns;
  set<Rule> rules, saferules, riskyrules;
  read_rif_rules(rif_rules, repl_pragmas, no_repl_pragmas, arb_pragmas, no_arb_pragmas);
  translate_repl_pragmas(repl_pragmas, repl_patterns);
  translate_repl_pragmas(no_repl_pragmas, no_repl_patterns);
  translate_repl_pragmas(arb_pragmas, arb_patterns);
  translate_repl_pragmas(no_arb_pragmas, no_arb_patterns);
  translate_rif_rules(rif_rules, rules);
  classify_rules(rules, repl_patterns, saferules, riskyrules, no_repl_patterns, arb_patterns, no_arb_patterns);
  print_cnf(saferules, riskyrules, repl_patterns, no_repl_patterns, arb_patterns, no_arb_patterns);
  ts_end = TIME();
  cerr << "[TIME] " << TIMEOUTPUT(DIFFTIME(ts_end, ts_begin)) << endl;
}
