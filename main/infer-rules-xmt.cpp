// c++ -I ../ -I ~/installs/include/ -DDATA_SIZE=17472 -DRULE_SIZE=464 -DTUPLE_SIZE=4 -DNUM_INDEX_STREAMS=100 -DNUM_RELATION_STREAMS=100 infer-rules-xmt.cpp
// mtarun a.out testfiles/minrdfs.enc testfiles/foaf.der testfiles/foaf-closure.der

#include <algorithm>
#include <cmath>
#include <ctime>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <set>
#ifndef FAKE
#include <snapshot/client.h>
#endif
#include <sstream>
#include <string>
#ifdef FAKE
#include <sys/stat.h>
#endif
#include <utility>
#include <vector>
#include "main/encode.h"
#ifndef FAKE
#include "mtgl/dynamic_array.hpp"
#include "mtgl/merge_sort.hpp"
#endif
#ifdef FAKE
#include "sys/endian.h"
#include "util/funcs.h"
#endif

#ifndef CONTAINER
#define CONTAINER list
#endif

#ifndef TUPLE_SIZE
#error "Must specify the tuple size (maximum number of variables in a rule) using -DTUPLE_SIZE."
#endif

#ifndef NUM_ATOM_PREDS
#error "Must specify the (maximum) number of atom predicates using -DNUM_ATOM_PREDS."
#endif

#ifdef DEBUG
#undef DEBUG
#endif
//#define DEBUG(msg, val) cerr << "[DEBUG] " << __FILE__ << ':' << __LINE__ << ": " << (msg) << (val) << endl
#define DEBUG(msg, val)

#ifndef FAKE
using namespace mtgl;
#endif
using namespace std;
#ifdef FAKE
using namespace sys;
using namespace util;
#endif

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

#pragma mta parallel off

template<typename T>
ostream &operator<<(ostream &stream, std::pair<T, T> &p) {
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
  range_of<std::pair<Term, Term> > slots;
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
  vector<std::pair<Term, Term> > slots;
  const uint8_t *offset = build_term(begin, end, frame.object);
  while (offset != NULL && offset != end) {
    std::pair<Term, Term> slot;
    offset = build_term(offset, end, slot.first);
    offset = build_term(offset, end, slot.second);
    slots.push_back(slot);
  }
  frame.slots.begin = new std::pair<Term, Term>[slots.size()];
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


////// JOIN PROCESSING //////

#pragma mta parallel on

#ifdef DEBUG_INFER
#undef DEBUG
#define DEBUG(msg, val) cerr << "[DEBUG] " << __FILE__ << ':' << __LINE__ << ": " << (msg) << (val) << endl
#endif

#ifdef FAKE
#define SNAP_ANY_SW 0

typedef struct stat snap_stat_buf;

void snap_init() {}

void snap_stat(const char *filename, uint64_t fsw, snap_stat_buf *file_stat, int64_t *snap_error) {
  stat(filename, file_stat);
}

void snap_restore(const char *filename, void *p, const size_t len, int64_t *snap_error) {
  ifstream fin (filename);
  fin.read((char *)p, len);
  fin.close();
}

void snap_snapshot(const char *filename, void *buffer, const size_t len, int64_t *snap_error) {
  ofstream fout (filename);
  fout.write((char *)buffer, len);
  fout.close();
}

#define purge(p) (*(p) = 0)
#define readfe(p) (*(p))
#define readff(p) (*(p))
#define writexf(p, v) (*(p) = (v))
#define writeef(p, v) (*(p) = (v))
#define int_fetch_add(p, v) (*(p)); (*(p)) += v

template <typename T, typename Comparator>
void merge_sort(T* array, long size, Comparator comp)
{
  sort(array, array + size, comp);
}

template<typename T>
class dynamic_array {
private:
  mutable T *data;
  size_t len, cap;
public:
  typedef typename vector<T>::value_type value_type;
  typedef unsigned long size_type;
  dynamic_array(size_type size = 0)
      : len(size), cap(std::max(size, (size_type)1)) {
    this->data = (T*)malloc(cap*sizeof(T));
  }
  dynamic_array(const dynamic_array<T> &a)
      : len(a.len), cap(a.cap) {
    this->data = (T*)malloc(cap*sizeof(T));
    copy(a.data, a.data + len, this->data);
  }
  ~dynamic_array() { 
    free(this->data);
  }
  size_type size() const { return this->len; }
  void resize(size_type new_size) {
    if (new_size <= this->cap) {
      this->len = new_size;
    } else {
      this->data = (T*)realloc(this->data, new_size*sizeof(T));
      this->len = this->cap = new_size;
    }
  }
  bool empty() const { return this->len == 0; }
  void reserve(size_type new_size) {
    if (new_size < this->len) {
      this->len = new_size;
    }
    if (new_size > this->cap) {
      this->data = (T*)realloc(this->data, new_size*sizeof(T));
      this->cap = new_size;
    }
  }
  T &operator[](size_type i) { return this->data[i]; }
  const T&operator[](const size_type i) const { return this->data[i]; }
  size_type push_back(const T &key) {
    this->reserve(this->len + 1);
    this->data[this->len] = key;
    return this->len++;
  }
  size_type unsafe_push_back(const T &key) {
    if (this->len >= this->cap) {
      cerr << "[ERROR] Screwed by unsafe_push_back." << endl;
    }
    this->data[this->len] = key;
    return this->len++;
  }
  void swap(dynamic_array<T> &rhs) {
    std::swap(this->data, rhs.data);
    std::swap(this->len, rhs.len);
    std::swap(this->cap, rhs.cap);
  }
  void clear() { this->len = 0; }
  T *get_data() const { return this->data; }
  dynamic_array<T> &operator=(const dynamic_array<T> &rhs) {
    free(this->data);
    this->data = (T*)malloc(rhs.cap * sizeof(T));
    copy(rhs.data, rhs.data + rhs.len, this->data);
    this->cap = rhs.cap;
    this->len = rhs.len;
    return *this;
  }
};
#endif

inline
uint32_t hash_jenkins_one_at_a_time_iter(uint32_t &h, const uint8_t b) {
  h += b;
  h += (h << 10);
  h ^= (h >> 6);
  return h;
}

inline
uint32_t hash_jenkins_one_at_a_time_accum(uint32_t &h, const uint8_t *begin, const uint8_t *end) {
  #pragma mta loop serial
  for (; begin != end; ++begin) {
    h = hash_jenkins_one_at_a_time_iter(h, *begin);
  }
  return h;
}

inline
uint32_t hash_jenkins_one_at_a_time_end(uint32_t &h) {
  h += (h << 3);
  h ^= (h >> 11);
  h += (h << 15);
  return h;
}

uint32_t hash_jenkins_one_at_a_time(const uint8_t *begin, const uint8_t *end)
    throw() {
  uint32_t h = 0;
  hash_jenkins_one_at_a_time_accum(h, begin, end);
  return hash_jenkins_one_at_a_time_end(h);
}

uint8_t *read_all(const char *filename, size_t &len) {
  uint8_t *p = (uint8_t*)malloc(len);
  int64_t snap_error;
  snap_restore(filename, p, len, &snap_error);
  return p;
}

void load_rules(const char *filename, dynamic_array<Rule> &rules) {
  snap_stat_buf file_stat;
  int64_t snap_error;
  snap_stat(filename, SNAP_ANY_SW, &file_stat, &snap_error);
  size_t len = file_stat.st_size;
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

//void print_rules(vector<Rule> &rules) {
//  vector<Rule>::iterator it = rules.begin();
//  for (; it != rules.end(); ++it) {
//    cerr << "[RULE] " << *it << endl << endl;
//  }
//}





// NEW BEGIN

struct Triple {
  constint_t at[3];
};

Triple &init_triple(Triple &t, const constint_t c) {
  size_t i;
  #pragma mta max concurrency 3
  for (i = 0; i < 3; ++i) {
    t.at[i] = c;
  }
  return t;
}

inline
Triple &init_triple(Triple &t) {
  return init_triple(t, 0);
}

dynamic_array<Triple> triples;

struct Tuple {
  constint_t at[TUPLE_SIZE];
};

Tuple &init_tuple(Tuple &t, const constint_t c) {
  size_t i;
  #pragma mta max concurrency TUPLE_SIZE
  for (i = 0; i < TUPLE_SIZE; ++i) {
    t.at[i] = c;
  }
  return t;
}

inline
Tuple &init_tuple(Tuple &t) {
  return init_tuple(t, 0);
}

class Order {
private:
  dynamic_array<size_t> order;
  bool total_ordering;
public:
  Order() : total_ordering(true) {
    // do nothing
  }
  Order(size_t i1, size_t i2, size_t i3) : total_ordering(false) {
    this->order.reserve(3);
    order.unsafe_push_back(i1);
    order.unsafe_push_back(i2);
    order.unsafe_push_back(i3);
  }
  Order(const dynamic_array<size_t> &order)
      : order(order), total_ordering(true) {
    // do nothing
  }
  Order(const dynamic_array<size_t> &order, bool total)
      : order(order), total_ordering(total) {
    // do nothing
  }
  Order(const Order &copy) : order(copy.order), total_ordering(copy.total_ordering) {
    // do nothing
  }
  Order &operator=(const Order &rhs) {
    this->order = rhs.order;
    this->total_ordering = rhs.total_ordering;
    return *this;
  }
  bool operator()(const Tuple &t1, const Tuple &t2) const {
    size_t i;
    size_t max = this->order.size();
    for (i = 0; i < max; ++i) {
      int j = this->order[i];
      if (j < TUPLE_SIZE && t1.at[j] != t2.at[j]) {
        return t1.at[j] < t2.at[j];
      }
    }
    if (this->total_ordering) {
      for (i = 0; i < TUPLE_SIZE; ++i) {
        if (t1.at[i] != t2.at[i]) {
          return t1.at[i] < t2.at[i];
        }
      }
    }
    return false;
  }
  bool operator()(const Triple &t1, const Triple &t2) const {
    size_t i;
    size_t max = this->order.size();
    for (i = 0; i < max; ++i) {
      int j = this->order[i];
      if (j < 3 && t1.at[j] != t2.at[j]) {
        return t1.at[j] < t2.at[j];
      }
    }
    if (this->total_ordering) {
      for (i = 0; i < 3; ++i) {
        if (t1.at[i] != t2.at[i]) {
          return t1.at[i] < t2.at[i];
        }
      }
    }
    return false;
  }
};

class Hash {
private:
  dynamic_array<size_t> order;
  bool total_ordering;
public:
  Hash() : total_ordering(true) {
    // do nothing
  }
  Hash(size_t i1, size_t i2, size_t i3) : total_ordering(false) {
    this->order.reserve(3);
    order.unsafe_push_back(i1);
    order.unsafe_push_back(i2);
    order.unsafe_push_back(i3);
  }
  Hash(const dynamic_array<size_t> &order)
      : order(order), total_ordering(false) {
    // do nothing
  }
  Hash(const dynamic_array<size_t> &order, const bool total)
      : order(order), total_ordering(total) {
    // do nothing
  }
  Hash(const Hash &copy) : order(copy.order), total_ordering(copy.total_ordering) {
    // do nothing
  }
  Hash &operator=(const Hash &rhs) {
    this->order = rhs.order;
    this->total_ordering = rhs.total_ordering;
    return *this;
  }
  uint32_t operator()(const Tuple &t) const {
    uint32_t h = 0;
    size_t i;
    size_t max = this->order.size();
    #pragma mta loop serial
    for (i = 0; i < max; ++i) {
      size_t j = this->order[i];
      constint_t val;
      if (j < TUPLE_SIZE) {
        val = t.at[j];
      } else {
        val = 0;
      }
      h = hash_jenkins_one_at_a_time_accum(h, (const uint8_t *)&val, (const uint8_t *)((&val) + 1));
    }
    if (this->total_ordering) {
      #pragma mta loop serial
      for (i = 0; i < TUPLE_SIZE; ++i) {
        h = hash_jenkins_one_at_a_time_accum(h, (const uint8_t *)&t.at[i], (const uint8_t *)((&t.at[i]) + 1));
      }
    }
    return hash_jenkins_one_at_a_time_end(h);
  }
  uint32_t operator()(const Triple &t) const {
    uint32_t h = 0;
    size_t i;
    size_t max = this->order.size();
    #pragma mta loop serial
    for (i = 0; i < max; ++i) {
      size_t j = this->order[i];
      constint_t val;
      if (j < 3) {
        val = t.at[j];
      } else {
        val = 0;
      }
      h = hash_jenkins_one_at_a_time_accum(h, (const uint8_t *)&val, (const uint8_t*)((&val) + 1));
    }
    if (this->total_ordering) {
      #pragma mta loop serial
      for (i = 0; i < 3; ++i) {
        h = hash_jenkins_one_at_a_time_accum(h, (const uint8_t *)&t.at[i], (const uint8_t *)((&t.at[i]) + 1));
      }
    }
    return hash_jenkins_one_at_a_time_end(h);
  }
};

// NEW END

template<typename T, typename CMP>
void unique(dynamic_array<T> &input, CMP cmp, dynamic_array<T> &output);

void load_data(const char *filename) {
  size_t i, j;
  snap_stat_buf file_stat;
  int64_t snap_error;
  snap_stat(filename, SNAP_ANY_SW, &file_stat, &snap_error);
  size_t numids = file_stat.st_size / sizeof(constint_t);
  dynamic_array<constint_t> ids;
  ids.resize(numids);
  constint_t *ids_data = ids.get_data();
  // XMT is big endian, so don't worry about it.
  snap_restore(filename, ids_data, file_stat.st_size, &snap_error);
#ifdef FAKE
  if (is_little_endian()) {
    for (i = 0; i < numids; ++i) {
      reverse_bytes(ids_data[i]);
    }
  }
#endif
  size_t numtriples = numids / 3;
  triples.resize(numtriples);
  Triple *triples_data = triples.get_data();
  #pragma mta assert parallel
  for (i = 0; i < numtriples; ++i) {
    size_t offset = 3*i;
    #pragma mta max concurrency 3
    for (j = 0; j < 3; ++j) {
      triples_data[i].at[j] = ids_data[offset+j];
    }
  }
  //unique(triples, Order(), triples);
#if 0
  for (i = 0; i < numtriples; ++i) {
    cerr << "LOADED ";
    for (j = 0; j < 3; ++j) {
      cerr << triples_data[i].at[j] << ' ' << endl;
    }
    cerr << endl;
  }
#endif
}

typedef set<varint_t> VarSet; // USE ONLY IN SERIAL CONTEXT

typedef dynamic_array<Tuple> Relation;
//#define Relation dynamic_array<Tuple>

#if 0
void print_relation(const Relation &r) {
  size_t i, j;
  cerr << "RELATION" << endl;
  for (i = 0; i < r.size(); ++i) {
    cerr << "TUPLE";
    for (j = 0; j < TUPLE_SIZE; ++j) {
      cerr << ' ' << r[i].at[j];
    }
    cerr << endl;
  }
}
#endif

Relation atoms[NUM_ATOM_PREDS];
constint_t atom_preds[NUM_ATOM_PREDS];
uint64_t found_atoms = 0;

void minusrel(Relation &intermediate, Relation &negated, Relation &result) {
  cerr << "[ERROR] Negation of non-special formulas is currently unsupported." << endl;
}


template<typename T, typename CMP>
void unique(dynamic_array<T> &input, CMP cmp, dynamic_array<T> &output) {
  bool sorted = true;
  bool uniq = true;
  size_t i;
  T *input_data = input.get_data();
  size_t input_size = input.size();
  for (i = 1; i < input_size; ++i) {
    sorted &= !cmp(input_data[i], input_data[i-1]);
    uniq &= cmp(input_data[i-1], input_data[i]);
  }
  if (uniq) {
    if (&input != &output) {
      output = input;
    }
    return;
  }
  if (!sorted) {
    merge_sort(input_data, input_size, cmp);
  }
  size_t *pos = (size_t*)malloc(input_size * sizeof(size_t));
  pos[0] = 0;
  #pragma mta assert parallel
  for (i = 1; i < input_size; ++i) {
    pos[i] = cmp(input_data[i-1], input_data[i]) ? 0 : 1;
  }
  #pragma mta loop recurrence
  for (i = 1; i < input_size; ++i) {
    pos[i] += pos[i - 1];
  }
  dynamic_array<T> temp;
  temp.resize(input_size - pos[input_size-1]);
  T *temp_data = temp.get_data();
  temp_data[0] = input_data[0];
  #pragma mta assert parallel
  for (i = 1; i < input_size; ++i) {
    temp_data[i-pos[i]] = input_data[i];
  }
  output.swap(temp);
  free(pos);
}

#ifdef DEBUG_JOIN
#undef DEBUG
#define DEBUG(a, b) cerr << a << b << endl
#endif

double LN2 = 0.69314718056;

#if 0
#include <bitset>
void print_bloom_filter(dynamic_array<uint64_t> &bloom) {
  cerr << "BLOOM FILTER ";
  size_t bloom_size = bloom.size();
  uint64_t *bloom_data = bloom.get_data();
  uint64_t i;
  for (i = 0; i < bloom_size; ++i) {
    cerr << bitset<64>(bloom_data[i]) << ' ';
  }
  cerr << endl;
}
#endif

void make_bloom_filter(Relation &tuples, Hash hash, size_t nitems, dynamic_array<uint64_t> &output) {
  size_t nbits = nitems / LN2 + 1;
  size_t nwords = (nbits >> 6) + 1;
  nbits = (nwords << 6);
  uint64_t mask = UINT64_C(0x3F); // bottom six bits
  //cerr << "BLOOM SPECS nwords=" << nwords << " nbits=" << nbits << " (should be " << nwords*64 << ") mask=" << bitset<64>(mask) << endl;
  dynamic_array<uint64_t> bloom;
  bloom.resize(nwords);
  size_t i, j;
  size_t numtuples = tuples.size();
  Tuple *tuple_data = tuples.get_data();
  uint64_t *bloom_data = bloom.get_data();
  #pragma mta assert parallel
  for (i = 0; i < nwords; ++i) {
    bloom_data[i] = 0;
  }
  uint32_t h = 0;
  #pragma mta assert parallel
  for (i = 0; i < numtuples; ++i) {
    h = hash(tuple_data[i]) % nbits;
    size_t wordi = (h >> 6);
    size_t biti = h & mask;
    uint64_t bit = UINT64_C(1) << biti;
    #pragma mta update
    bloom_data[wordi] = bloom_data[wordi] | bit;
    //cerr << "h=" << h << " wordi=" << wordi << " biti=" << biti << " bit=" << bitset<64>(bit) << " bloom_data[wordi]=" << bitset<64>(bloom_data[wordi]) << endl;
  }
  //print_bloom_filter(bloom);
  output.swap(bloom);
  //print_bloom_filter(output);
}

void use_bloom_filter(Relation &tuples, Hash hash, dynamic_array<uint64_t> &bloom, Relation &output) {
  size_t ntuples = tuples.size();
  Tuple *tuples_data = tuples.get_data();
  Relation filtered;
  filtered.reserve(ntuples);
  uint64_t *bloom_data = bloom.get_data();
  size_t nbits = bloom.size() * (sizeof(uint64_t) << 3);
  uint64_t mask = UINT64_C(0x3F); // bottom six bits
  size_t i;
  #pragma mta assert parallel
  for (i = 0; i < ntuples; ++i) {
    uint32_t h = hash(tuples_data[i]) % nbits;
    size_t wordi = (h >> 6);
    size_t biti = h & mask;
    uint64_t bit = UINT64_C(1) << biti;
    if ((bloom_data[wordi] & bit) != 0) {
      filtered.unsafe_push_back(tuples_data[i]);
    }
  }
  output.swap(filtered);
}

void join(Relation &lhs, Relation &rhs, dynamic_array<size_t> &vars, Relation &output) {
  Relation results;
  Hash hash(vars);
  dynamic_array<uint64_t> lbloom, rbloom;
  size_t nitems = std::max(lhs.size(), rhs.size());
  make_bloom_filter(lhs, hash, nitems, lbloom);
  make_bloom_filter(rhs, hash, nitems, rbloom);
  size_t bloom_size = lbloom.size();
  uint64_t *lbloom_data = lbloom.get_data();
  uint64_t *rbloom_data = rbloom.get_data();
  size_t i, j, k;
  #pragma mta assert parallel
  for (i = 0; i < bloom_size; ++i) {
    lbloom_data[i] &= rbloom_data[i];
  }
  //print_bloom_filter(lbloom);
  Relation flhs, frhs;
  use_bloom_filter(lhs, hash, lbloom, flhs);
  //cerr << "LHS filtered from " << lhs.size() << " to " << flhs.size() << " items." << endl;
  use_bloom_filter(rhs, hash, rbloom, frhs);
  //cerr << "RHS filtered from " << rhs.size() << " to " << frhs.size() << " items." << endl;
  Order order(vars, false);
  if (flhs.size() < frhs.size()) {
    flhs.swap(frhs);
  }
  size_t lsize = flhs.size();
  size_t rsize = frhs.size();
  Tuple *ldata = flhs.get_data();
  Tuple *rdata = frhs.get_data();
  Tuple **lower_bounds = (Tuple**)malloc(lsize*sizeof(Tuple*));
  uint64_t *offsets = (uint64_t *)malloc(lsize*sizeof(uint64_t));
  merge_sort(rdata, rsize, order);
  #pragma mta assert parallel
  for (i = 0; i < lsize; ++i) {
    lower_bounds[i] = lower_bound(rdata, rdata + rsize, ldata[i], order);
    Tuple *upper = upper_bound(lower_bounds[i], (Tuple *)rdata + rsize, ldata[i], order);
    offsets[i] = upper - lower_bounds[i];
    //cerr << "offsets[" << i << "] = " << offsets[i] << endl;
  }
  #pragma mta loop recurrence
  for (i = 1; i < lsize; ++i) {
    offsets[i] += offsets[i-1];
    //cerr << "offsets[" << i << "] = " << offsets[i] << endl;
  }
  results.resize(offsets[lsize-1]);
  Tuple *results_data = results.get_data();
  #pragma mta assert parallel
  for (i = 0; i < lsize; ++i) {
    Tuple *lower = lower_bounds[i];
    size_t offset, maxj;
    if (i == 0) {
      offset = 0;
      maxj = offsets[0];
    } else {
      offset = offsets[i-1];
      maxj = offsets[i] - offset;
    }
    #pragma mta assert parallel
    for (j = 0; j < maxj; ++j) {
      uint64_t loc = int_fetch_add(&offset, 1);
      #pragma mta max concurrency TUPLE_SIZE
      for (k = 0; k < TUPLE_SIZE; ++k) {
        // max is just picking whichever is not a variable
        // if such a value exists.  If they are both non-zero,
        // then they are the same values.
        results_data[loc].at[k] = std::max(ldata[i].at[k], lower[j].at[k]);
        //cerr << "results_data[" << loc << "].at[" << k << "] = std::max(" << ldata[i].at[k] << ", " << lower[j].at[k] << ");" << endl;
      }
    }
  }
  free(lower_bounds);
  free(offsets);
  //print_relation(results);
  output.swap(results);
  //print_relation(output);
}
#ifdef DEBUG_JOIN
#undef DEBUG
#define DEBUG(a, b)
#endif

bool special(Condition &condition, Relation &intermediate, Relation &filtered, bool sign) {
  if (condition.type != ATOMIC) {
    return false;
  }
  Term *lhs, *rhs;
  switch (condition.get.atom.type) {
    case ATOM:
    case MEMBERSHIP:
    case SUBCLASS:
    case FRAME:
      return false;
    case EQUALITY:
      lhs = &(condition.get.atom.get.sides[0]);
      rhs = &(condition.get.atom.get.sides[1]);
      if (lhs->type == VARIABLE && rhs->type == CONSTANT) {
        swap(lhs, rhs);
      }
      if (rhs->type == CONSTANT) {
        if (( sign && lhs->get.constant != rhs->get.constant) ||
            (!sign && lhs->get.constant == rhs->get.constant)) {
          Relation empty;
          filtered.swap(empty);
        } else {
          filtered = intermediate;
        }
      } else if (lhs->type == CONSTANT) {
        Tuple *inter_data = intermediate.get_data();
        size_t inter_size = intermediate.size();
        Relation results;
        results.reserve(inter_size);
        size_t i;
        #pragma mta assert parallel
        for (i = 0; i < inter_size; ++i) {
          Tuple &tuple = inter_data[i];
          if (tuple.at[rhs->get.variable] == 0) {
            if (sign) {
              Tuple t = tuple;
              t.at[rhs->get.variable] = lhs->get.constant;
              results.unsafe_push_back(t);
            } else {
              cerr << "[ERROR] Variables in inequality should be bound by atomics formulas on the left." << endl;
            }
          } else if (( sign && tuple.at[rhs->get.variable] == lhs->get.constant) ||
                     (!sign && tuple.at[rhs->get.variable] != lhs->get.constant)) {
            results.unsafe_push_back(tuple);
          }
        }
        filtered.swap(results);
      } else {
        Tuple *inter_data = intermediate.get_data();
        size_t inter_size = intermediate.size();
        Relation results;
        results.reserve(inter_size);
        size_t i;
        #pragma mta assert parallel
        for (i = 0; i < inter_size; ++i) {
          Tuple &tuple = inter_data[i];
          constint_t c1 = tuple.at[lhs->get.variable];
          constint_t c2 = tuple.at[rhs->get.variable];
          if (c1 == 0 && c2 == 0) {
            cerr << "[ERROR] Please make sure (in)equality statements occur to the right of atomic formulas that bind a variable in the (in)equality." << endl;
          } else if (!sign && (c1 == 0 || c2 == 0)) {
            cerr << "[ERROR] Please make sure that inequality statements occur to the right of atomic formulas that bind BOTH variables in the inequality." << endl;
          } else if (c1 == 0) {
            Tuple t = tuple;
            t.at[lhs->get.variable] = c2;
            results.unsafe_push_back(t);
          } else if (c2 == 0) {
            Tuple t = tuple;
            t.at[rhs->get.variable] = c1;
            results.unsafe_push_back(t);
          } else if (sign && c1 == c2) {
            results.unsafe_push_back(tuple);
          } else if (!sign && c1 != c2) {
            results.unsafe_push_back(tuple);
          }
        }
        filtered.swap(results);
      }
      return true;
    case EXTERNAL:
      break;
    default:
      cerr << "[ERROR] Unhandled case " << (int)condition.get.atom.type << " at line " << __LINE__ << endl;
      return false;
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
          #pragma mta loop serial
          for (; t != arg1->get.termlist.end; ++t) {
            if (t->type == CONSTANT && t->get.constant == arg2->get.constant) {
              filtered = intermediate;
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
      Tuple *inter_data = intermediate.get_data();
      size_t inter_size = intermediate.size();
      Relation results;
      results.reserve(inter_size);
      size_t i;
      for (i = 0; i < inter_size; ++i) {
        Tuple &tuple = inter_data[i];
        constint_t c = tuple.at[arg2->get.variable];
        if (c == 0) {
          cerr << "[ERROR] Variables in built-ins must already be bound by atomic formulas to the left." << endl;
        } else {
          Term *t = arg1->get.termlist.begin;
          #pragma mta loop serial
          for (; t != arg1->get.termlist.end; ++t) {
            if (t->type == CONSTANT && t->get.constant == c) {
              if (sign) {
                results.unsafe_push_back(tuple);
              }
              break;
            }
          }
          if (!sign && t == arg1->get.termlist.end) {
            results.unsafe_push_back(tuple);
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

inline
bool special(Condition &condition, Relation &intermediate, Relation &filtered) {
  return special(condition, intermediate, filtered, true);
}

// called only in a serial context
void query_atom(Atom &atom, VarSet &allvars, Relation &results) {
  // just doing a scan... great for XMT, bad for others.
  size_t i, j;
  size_t max = atom.arguments.end - atom.arguments.begin;
  VarSet newvars;
  size_t nvars = 0;
  Term *term = atom.arguments.begin;
  #pragma mta loop serial
  for (; term != atom.arguments.end; ++term) {
    if (term->type == VARIABLE) {
      allvars.insert(term->get.variable);
      newvars.insert(term->get.variable);
      ++nvars;
    } else if (term[i].type != CONSTANT) {
      cerr << "[ERROR] Not handling lists or functions in atoms.  Results will be incorrect." << endl;
    }
  }
  if (nvars > newvars.size()) {
    cerr << "[ERROR] Not handling repeated variables in atoms." << endl;
    return;
  }
  for (i = 0; i < found_atoms && atom_preds[i] != atom.predicate; ++i) {
    // just finding the index into atoms
    // horribly inefficienct, but okay for my evaluation because
    // there are very few atoms... like maybe 10
  }
  if (i == found_atoms) {
    if (found_atoms >= NUM_ATOM_PREDS) {
      cerr << "[ERROR] There are more atom predicates than was specified with NUM_ATOM_PREDS.  Fail." << endl;
      return;
    }
    i = int_fetch_add(&found_atoms, 1); // should be serial, but just in case
    atom_preds[i] = atom.predicate;
  }
  //print_relation(atoms[i]);
  Tuple *base_data = atoms[i].get_data();
  size_t base_size = atoms[i].size();
  Relation temp;
  temp.reserve(base_size);
  #pragma mta assert parallel
  for (i = 0; i < base_size; ++i) {
    Tuple &tuple = base_data[i];
    Tuple result; init_tuple(result);
    Term *t = atom.arguments.begin;
    size_t max = atom.arguments.end - t;
    for (j = 0; j < max; ++j) {
      if (t[j].type == VARIABLE) {
        result.at[t[j].get.variable] = tuple.at[j];
      } else if (t[j].type == CONSTANT) {
        if (tuple.at[j] != t[j].get.constant) {
          break;
        }
      }
    }
    if (j == max) {
      temp.unsafe_push_back(result);
    }
  }
  //print_relation(temp);
  results.swap(temp);
  //print_relation(results);
  //cerr << "[DEBUG] Selected " << results.size() << " atoms (not triples)." << endl;
}

// called only in a serial context
void query(Atomic &atom, VarSet &allvars, Relation &results) {
  //cerr << "Beginning query with atomic type " << (int)atom.type << endl;
  switch (atom.type) {
    case ATOM:
      query_atom(atom.get.atom, allvars, results);
      return;
    case MEMBERSHIP:
    case SUBCLASS:
    case EQUALITY:
      return;
    case EXTERNAL:
      cerr << "[ERROR] This should never happen, at line " << __LINE__ << endl;
      return;
    case FRAME:
      break; // just handle outside this messy switch
    default:
      cerr << "[ERROR] Unhandled case " << (int)atom.type << " at line " << __LINE__ << endl;
      return;
  }
  if (atom.get.frame.slots.end - atom.get.frame.slots.begin != 1) {
    cerr << "[ERROR] Not handling frames unless they have exactly one slot." << endl;
    return;
  }
  //cerr << "Triple pattern creation...";
  Term *triple_pattern[3];
  triple_pattern[0] = &(atom.get.frame.object);
  triple_pattern[1] = &(atom.get.frame.slots.begin->first);
  triple_pattern[2] = &(atom.get.frame.slots.begin->second);
  //cerr << " done." << endl;
  size_t i, j;
  #pragma mta loop serial
  for (i = 0; i < 3; ++i) {
    if (triple_pattern[i]->type == VARIABLE) {
      allvars.insert(triple_pattern[i]->get.variable);
      //cerr << __LINE__ << " triple_pattern[" << i << "] = var " << (int)triple_pattern[i]->get.variable << endl;
    } //else {
      //cerr << __LINE__ << " triple_pattern[" << i << "] = const " << (int)triple_pattern[i]->get.constant << endl;
    //}
  }
  //cerr << "And collected the new variables." << endl;
  for (i = 0; i < 3; ++i) {
    if (triple_pattern[i]->type == LIST) {
      results.clear();
      return; // not an error; there are no such list in RDF, so results are empty.
    } else if (triple_pattern[i]->type == FUNCTION) {
      cerr << "[ERROR] Functions are not supported." << endl;
      return;
    }
  }
  Triple *triples_data = triples.get_data();
  size_t numtriples = triples.size();
  dynamic_array<size_t> nums;
  //cerr << __LINE__ << " numtriples=" << numtriples << endl;
  nums.resize(numtriples);
  //cerr << __LINE__ << " nums.size()=" << nums.size() << endl;
  #pragma mta assert parallel
  for (i = 0; i < numtriples; ++i) {
    for (j = 0; j < 3; ++j) {
      //if (triple_pattern[j]->type == CONSTANT) cerr << "[" << j << "] " << triple_pattern[j]->get.constant << " != " << triples_data[i].at[j] << " ?" << endl;
      if (triple_pattern[j]->type == CONSTANT &&
          triple_pattern[j]->get.constant != triples_data[i].at[j]) {
        break;
      }
    }
    nums[i] = j == 3 ? 1 : 0;
    //cerr << __LINE__ << ": nums[" << i << "] = " << nums[i] << endl;
  }
  //cerr << "Got my markings." << endl;
  #pragma mta loop recurrence
  for (i = 1; i < numtriples; ++i) {
    nums[i] += nums[i-1];
    //cerr << __LINE__ << ": nums[" << i << "] = " << nums[i] << endl;
  }
  //cerr << "And my partial sums." << endl;
  Relation intermediate;
  //cerr << "... okay, made it to line " << __LINE__ << endl;
  intermediate.resize(nums[numtriples-1]);
  //cerr << "... okay, made it to line " << __LINE__ << " where intermediate.size()=" << intermediate.size() << " (should be " << nums[numtriples-1] << ")" << endl;
  Tuple *inter_data = intermediate.get_data();
  //cerr << "... okay, made it to line " << __LINE__ << endl;
  if (nums[0] == 1) {
    init_tuple(inter_data[0]);
    for (j = 0; j < 3; ++j) {
      if (triple_pattern[j]->type == VARIABLE) {
        inter_data[0].at[triple_pattern[j]->get.variable] = triples_data[0].at[j];
      }
    }
  }
  //cerr << "... okay, made it to line " << __LINE__ << endl;
  #pragma mta assert parallel
  for (i = 1; i < numtriples; ++i) {
    //cerr << "i=" << i << " prev=" << nums[i-1] << " nums[i]=" << nums[i] << endl;
    if (nums[i] > nums[i-1]) {
      //cerr << "init_tuple... ";
      init_tuple(inter_data[nums[i-1]]);
      //cerr << "passed." << endl;
      #pragma mta max concurrency 3
      for (j = 0; j < 3; ++j) {
        if (triple_pattern[j]->type == VARIABLE) {
          //cerr << "inter_data[" << nums[i-1] << "].at[" << (int)triple_pattern[j]->get.variable << "] = " << triples_data[i].at[j] << endl;
          inter_data[nums[i-1]].at[triple_pattern[j]->get.variable] = triples_data[i].at[j];
        }
      }
    }
  }
  //cerr << "... okay, made it to line " << __LINE__ << endl;
  results.swap(intermediate);
  //cerr << "Number selected = " << results.size() << endl;
}

// called only in a serial context
void query(Condition &condition, VarSet &allvars, Relation &results) {
  DEBUG("Beginning query with condition type ", (int)condition.type);
  deque<void*> negated;
  Relation intermediate;
  switch (condition.type) {
    case ATOMIC: {
      query(condition.get.atom, allvars, results);
      return;
    }
    case CONJUNCTION: {
      Condition *subformula = condition.get.subformulas.begin;
      #pragma mta loop serial
      for (; subformula != condition.get.subformulas.end; ++subformula) {
        //cerr << "Subformula " << (subformula - condition.get.subformulas.begin) << endl;
        if (subformula->type == NEGATION) {
          negated.push_back((void*)subformula->get.subformulas.begin);
          continue;
        }
        Relation subresult;
        VarSet newvars;
        // TODO vvv this won't work right if a non-special query
        // has not been performed first
        if (special(*subformula, intermediate, subresult)) {
          // TODO need to collect newvars
          if (subformula == condition.get.subformulas.begin) {
            cerr << "[ERROR] Must have non-special query at beginning of conjunction." << endl;
          }
          if (subresult.empty()) {
            results.swap(subresult);
            return;
          }
          intermediate.swap(subresult);
          continue;
        }
        query(*subformula, newvars, subresult);
        if (subresult.empty()) {
          results.swap(subresult);
          return;
        }
        if (subformula == condition.get.subformulas.begin) {
          intermediate.swap(subresult);
        } else {
          dynamic_array<size_t> joinvars;
          joinvars.reserve(TUPLE_SIZE);
          size_t rank;
          VarSet::iterator it = newvars.begin();
          VarSet::iterator end = newvars.end();
          #pragma mta loop serial 
          for (; it != end; ++it) {
            if (allvars.count(*it) > 0) {
              joinvars.unsafe_push_back(*it);
            }
          }
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
      cerr << "[ERROR] Existential formulas are not supported." << endl;
      return;
    }
    case NEGATION: {
      cerr << "[ERROR] Rule body is not allowed to be a negated formula." << endl;
      return;
    }
    default: {
      cerr << "[ERROR] Unhandled case " << (int)condition.type << " on line " << __LINE__ << endl;
      return;
    }
  }
  deque<void*>::iterator it = negated.begin();
  #pragma mta loop serial
  for (; !intermediate.empty() && it != negated.end(); ++it) {
    Relation negresult;
    Condition *cond = (Condition*) *it;
    if (special(*cond, intermediate, negresult, false)) {
      intermediate.swap(negresult);
      continue;
    }
    query(*((Condition*)(*it)), allvars, negresult);
    minusrel(intermediate, negresult, intermediate);
  }
  results.swap(intermediate);
}

template<typename T, typename CMP>
size_t set_union_unique(dynamic_array<T> &base, dynamic_array<T> &tuples, CMP cmp) {
  //cerr << "SET_UNION_UNIQUE BEGIN" << endl;
  //cerr << "  * before calling unique, base_size = " << base.size() << endl;
  unique(base, cmp, base);
  unique(tuples, cmp, tuples);
  T *base_data = base.get_data();
  size_t base_size = base.size();
  //cerr << "  * after calling unique, base_size = " << base_size << endl;
  T *tuples_data = tuples.get_data();
  size_t tuples_size = tuples.size();
  size_t *selected = (size_t*)malloc(tuples_size*sizeof(size_t));
  size_t i;
  #pragma mta assert parallel
  for (i = 0; i < tuples_size; ++i) {
    T *lower = lower_bound(base_data, base_data + base_size, tuples_data[i], cmp);
    selected[i] = (lower == base_data + base_size || cmp(tuples_data[i], *lower)) ? 1 : 0;
    //cerr << __LINE__ << " selected[" << i << "] = " << selected[i] << endl;
  }
  #pragma mta loop recurrence
  for (i = 1; i < tuples_size; ++i) {
    selected[i] += selected[i-1];
    //cerr << __LINE__ << " selected[" << i << "] = " << selected[i] << endl;
  }
  base.resize(base_size + selected[tuples_size - 1]);
  base_data = base.get_data();
  if (selected[0] == 1) {
    base_data[base_size] = tuples_data[0];
  }
  #pragma mta assert parallel
  for (i = 1; i < tuples_size; ++i) {
    if (selected[i-1] < selected[i]) {
      base_data[base_size+selected[i-1]] = tuples_data[i];
    }
  }
  free(selected);
  return base_size;
  //cerr << "SET_UNION_UNIQUE END" << endl;
}

size_t act(Atom &atom, Relation &results) {
  //cerr << "ACTING BASED ON "; print_relation(results);
  size_t pred_loc, i, j;
  for (pred_loc = 0; pred_loc < found_atoms && atom_preds[pred_loc] != atom.predicate; ++pred_loc) {
    // just finding the index into atoms
    // horribly inefficienct, but okay for my evaluation because
    // there are very few atoms... like maybe 10
  }
  if (pred_loc == found_atoms) {
    if (found_atoms >= NUM_ATOM_PREDS) {
      cerr << "[ERROR] There are more atom predicates than was specified with NUM_ATOM_PREDS.  Fail." << endl;
      return 0;
    }
    pred_loc = int_fetch_add(&found_atoms, 1); // should be serial, but just in case
    atom_preds[pred_loc] = atom.predicate;
  }
  Order lexical_order;
  Tuple *results_data = results.get_data();
  size_t results_size = results.size();
  Relation tuples;
  tuples.resize(results_size);
  Tuple *tuples_data = tuples.get_data();
  size_t nargs = atom.arguments.end - atom.arguments.begin;
  #pragma mta assert parallel
  for (i = 0; i < results_size; ++i) {
    init_tuple(tuples_data[i]);
    for (j = 0; j < nargs; ++j) {
      Term &arg = atom.arguments.begin[j];
      if (arg.type == CONSTANT) {
        tuples_data[i].at[j] = arg.get.constant;
      } else if (arg.type == VARIABLE) {
        tuples_data[i].at[j] = results_data[i].at[arg.get.variable];
      } else {
        cerr << "[ERROR] Lists and/or function in target atom not currently supported." << endl;
      }
    }
  }
  Relation &base = atoms[pred_loc];
  //cerr << "BASE "; print_relation(base);
  //cerr << "NEW "; print_relation(tuples);
  //cerr << "PREDICATE " << hex << (int)atom.predicate << dec << endl;
  size_t old_size = set_union_unique(base, tuples, lexical_order);
  //cerr << "old_size = " << old_size << endl;
  //cerr << "UNIONED "; print_relation(base);
  //cerr << "base.size() = " << base.size() << endl;
  return base.size() - old_size;
}


size_t act(ActionBlock &action_block, Relation &results) {
  if (results.empty()) {
    return 0;
  }
  if (action_block.action_variables.begin != action_block.action_variables.end) {
    cerr << "[ERROR] Action variables are not supported." << endl;
    return 0;
  }
  Tuple *results_data = results.get_data();
  size_t results_size = results.size();
  Action *action = action_block.actions.begin;
  size_t nasserts = 0;
  #pragma mta loop serial
  for (; action != action_block.actions.end; ++action) {
    switch (action->type) {
      case ASSERT_FACT: {
        if (action->get.atom.type == ATOM) {
          nasserts += act(action->get.atom.get.atom, results);
          continue;
        }
        if (action->get.atom.type != FRAME) {
          cerr << "[ERROR] For now, supporting only assertion of frames/triples/atoms." << endl;
          return 0;
        }
        Frame &frame = action->get.atom.get.frame;
        if (frame.slots.end - frame.slots.begin != 1) {
          cerr << "[ERROR] Frames in actions that don't have exactly one slot are not supported." << endl;
        }
        Term *triple_pattern[3];
        triple_pattern[0] = &frame.object;
        triple_pattern[1] = &frame.slots.begin->first;
        triple_pattern[2] = &frame.slots.begin->second;
        size_t i, j;
        for (i = 0; i < 3; ++i) {
          if (triple_pattern[i]->type == LIST || triple_pattern[i]->type == FUNCTION) {
            cerr << "[ERROR] Not support lists or functions in actions." << endl;
            return 0;
          }
        }
        dynamic_array<Triple> new_triples;
        new_triples.resize(results_size);
        Triple *new_triples_data = new_triples.get_data();
        #pragma mta assert parallel
        for (i = 0; i < results_size; ++i) {
          Triple &trip = new_triples_data[i];
          init_triple(trip);
          for (j = 0; j < 3; ++j) {
            if (triple_pattern[j]->type == CONSTANT) {
              trip.at[j] = triple_pattern[j]->get.constant;
            } else {
              trip.at[j] = results_data[i].at[triple_pattern[j]->get.variable];
            }
          }
        }
        size_t old_size = set_union_unique(triples, new_triples, Order());
        //cerr << "old triples size=" << old_size << " new triples size=" << triples.size() << endl;
        nasserts += (triples.size() - old_size);
        break;
      }
      default: {
        cerr << "[ERROR] Currently supporting only ASSERT_FACT." << endl;
        return 0;
      }
    }
  }
  return nasserts;
}

void infer(dynamic_array<Rule> &rules) {
  size_t rules_since_change = 0;
  size_t cycle_count;
  Rule *rules_data = rules.get_data();
  size_t rules_size = rules.size();
  #pragma mta loop serial
  for (cycle_count = 0; rules_since_change < rules_size; ++cycle_count) {
    cerr << "  Cycle " << (cycle_count+1) << "..." << endl;
    size_t rulecount;
    #pragma mta loop serial
    for (rulecount = 0; rulecount < rules_size && rules_since_change < rules_size; ++rulecount) {
      cerr << "    Rule " << (rulecount+1) << "..." << endl;
      size_t nasserts = 0;
      size_t local_nasserts = 1;
      size_t app_count;
      #pragma mta loop serial
      for (app_count = 0; local_nasserts > 0; ++app_count) {
        cerr << "      Application " << (app_count+1) << "... ";
        Relation results;
        VarSet allvars;
        cerr << "querying... ";
        query(rules_data[rulecount].condition, allvars, results);
        cerr << results.size() << " results... acting... ";
        local_nasserts = act(rules_data[rulecount].action_block, results);
        cerr << local_nasserts << " new assertions." << endl;
        nasserts += local_nasserts;
      }
      if (nasserts > 0) {
        rules_since_change = 0;
      } else {
        ++rules_since_change;
      }
    }
  }
}

void print_data(char *filename) {
  size_t triples_size = triples.size();
  Triple *triples_data = triples.get_data();
  constint_t *data = (constint_t*)malloc(3 * triples_size * sizeof(constint_t));
  size_t i, j;
  #pragma mta assert parallel
  for (i = 0; i < triples_size; ++i) {
    size_t offset = 3*i;
    #pragma mta max concurrency 3
    for (j = 0; j < 3; ++j) {
      data[offset+j] = triples_data[i].at[j];
#ifdef FAKE
      if (is_little_endian()) {
        reverse_bytes(data[offset+j]);
      }
#endif
    }
  }
  int64_t snap_error;
  snap_snapshot(filename, data, 3 * triples_size * sizeof(constint_t), &snap_error);
  free(data);
  for (i = 0; i < found_atoms; ++i) {
    if (atom_preds[i] == CONST_RIF_ERROR) {
      break;
    }
  }
  if (i < found_atoms && !atoms[i].empty()) {
    cerr << "INCONSISTENT" << endl;
  }
}

int main(int argc, char **argv) {
  snap_init();
  dynamic_array<Rule> rules;
  time_t time_start_loading_rules = time(NULL);
  cout << "Loading rules at time " << time_start_loading_rules << "... ";
  load_rules(argv[1], rules);
  cout << "loaded " << rules.size() << " rules." << endl;
  //print_rules(rules);
  time_t time_start_loading_data = time(NULL);
  cout << "Loading data at time " << time_start_loading_data << "... ";
  load_data(argv[2]);
  cout << "loaded " << triples.size() << " triples." << endl;
  time_t time_start_inferring = time(NULL);
  cout << "Inferring at time " << time_start_inferring << "..." << endl;
  infer(rules);
  time_t time_start_writing_data = time(NULL);
  cout << "Writing " << triples.size() << " triples at time " << time_start_writing_data << "..." << endl;
  print_data(argv[3]);
  time_t time_finish = time(NULL);
  cout << "DONE at time " << time_finish << "." << endl;
  cout << "Loading rules took " << difftime(time_start_loading_data, time_start_loading_rules) << endl;
  cout << "Loading data took " << difftime(time_start_inferring, time_start_loading_data) << endl;
  cout << "Inferring took " << difftime(time_start_writing_data, time_start_inferring) << endl;
  cout << "Writing data took " << difftime(time_finish, time_start_writing_data) << endl;
  cout << "OVERALL took " << difftime(time_finish, time_start_loading_rules) << endl;
  return 0;
}
