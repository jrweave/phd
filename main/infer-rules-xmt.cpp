// c++ -I ../ -I ~/installs/include/ -DDATA_SIZE=17472 -DRULE_SIZE=464 -DTUPLE_SIZE=4 -DNUM_INDEX_STREAMS=100 -DNUM_RELATION_STREAMS=100 infer-rules-xmt.cpp
// mtarun a.out testfiles/minrdfs.enc testfiles/foaf.der testfiles/foaf-closure.der

#include <algorithm>
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
#include <utility>
#include <vector>
#include "main/encode.h"
#ifndef FAKE
#include "mtgl/dynamic_array.hpp"
#endif
#ifdef FAKE
#include "sys/endian.h"
#include "util/funcs.h"
#endif

#ifndef CONTAINER
#define CONTAINER list
#endif

#ifndef DATA_SIZE
#error "Must specify the size of the data file (in bytes) using -DDATA_SIZE."
#endif

#ifndef RULE_SIZE
#error "Must specify the size of the rule file (in bytes) using -DRULE_SIZE."
#endif

#ifndef TUPLE_SIZE
#error "Must specify the tuple size (maximum number of variables in a rule) using -DTUPLE_SIZE."
#endif

#ifndef NUM_INDEX_STREAMS
#error "Must specify maximum number of streams for handling triple indexes using -DNUM_INDEX_STREAMS."
#endif

#ifndef NUM_RELATION_STREAMS
#error "Must specify maximum number of streams for handling relations (query results) using -DNUM_RELATION_STREAMS."
#endif

#ifndef NUM_VAR_SET_STREAMS
//#warning "Maximum number of streams for handling sets of variables was not specified using -DNUM_VAR_SET_STREAMS, so defaulting to the value specified for TUPLE_SIZE."
#define NUM_VAR_SET_STREAMS TUPLE_SIZE
#endif

#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG(msg, val)

#ifdef FAKE
#define FAKE_FOR_ALL_STREAMS(r, n, N) n = N; for (r = 0; r < n; ++r)
#else
#define FAKE_FOR_ALL_STREAMS(r, n, N)
#endif

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
void snap_init() {}

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

template<typename T>
class dynamic_array {
private:
  vector<T> array;
  T *real_array;
  bool changed;
public:
  typedef typename vector<T>::value_type value_type;
  typedef unsigned long size_type;
  dynamic_array(size_type size = 0) : real_array(NULL), changed(false) {}
  dynamic_array(const dynamic_array<T> &a) : array(a.array), real_array(NULL), changed(false) {}
  ~dynamic_array() { if (this->real_array == NULL) { delete [] this->real_array; } }
  size_type size() const { return this->array.size(); }
  void resize(size_type new_size) { this->changed = true; this->array.resize(new_size); }
  bool empty() const { return this->array.empty(); }
  void reserve(size_type new_size) { this->array.reserve(new_size); }
  T &operator[](size_type i) { this->changed = true; return this->array[i]; }
  size_type push_back(const T &key) { this->changed = true; this->array.push_back(key); return this->array.size() - 1; }
  void swap(dynamic_array<T> &rhs) { this->changed = true; this->array.swap(rhs.array); }
  void clear() { this->changed = true; this->array.clear(); }
  T *get_data() {
    if (this->changed) {
      delete[] this->real_array;
      this->real_array = NULL;
      this->changed = false;
    }
    if (this->real_array == NULL) {
      this->real_array = new T[this->array.size()];
      copy(this->array.begin(), this->array.end(), this->real_array);
    }
    return this->real_array;
  }
  dynamic_array<T> &operator=(const dynamic_array<T> &rhs) {
    this->changed = true;
    this->array = rhs.array;
    if (this->real_array != NULL) {
      delete[] this->real_array;
    }
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
  for (; begin != end; ++begin) {
    hash_jenkins_one_at_a_time_iter(h, *begin);
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
  size_t len;
  len = RULE_SIZE;
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

struct Index;

struct Tuple {
  constint_t at[TUPLE_SIZE];
};

struct Triple {
  constint_t at[3];
};

Tuple &init_tuple(Tuple &t, const constint_t c) {
  size_t i;
  #pragma mta assert parallel
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

bool operator<(const Tuple &lhs, const Tuple &rhs) {
  int i;
  for (i = 0; i < TUPLE_SIZE; ++i) {
    if (lhs.at[i] != rhs.at[i]) {
      return lhs.at[i] < rhs.at[i];
    }
  }
  return false;
}

bool operator==(const Tuple &lhs, const Tuple &rhs) {
  int i;
  for (i = 0; i < TUPLE_SIZE; ++i) {
    if (lhs.at[i] != rhs.at[i]) {
      return false;
    }
  }
  return true;
}

Triple &init_triple(Triple &t, const constint_t c) {
  size_t i;
  #pragma mta assert parallel
  #pragma mta max concurrency 3
  for (i = 0; i < 3; ++i) {
    t.at[i] = c;
  }
  return t;
}

Triple &init_triple(Triple &t) {
  return init_triple(t, 0);
}

bool operator<(const Triple &lhs, const Triple &rhs) {
  int i;
  for (i = 0; i < 3; ++i) {
    if (lhs.at[i] != rhs.at[i]) {
      return lhs.at[i] < rhs.at[i];
    }
  }
  return false;
}

bool operator==(const Triple &lhs, const Triple &rhs) {
  int i;
  for (i = 0; i < 3; ++i) {
    if (lhs.at[i] != rhs.at[i]) {
      return false;
    }
  }
  return true;
}

void purge_locks(int *locks, const size_t num) {
  size_t i;
  #pragma mta assert parallel
  for (i = 0; i < num; ++i) {
    purge(&locks[i]);
  }
}

void init_locks_full(int *locks, const size_t num, const int val) {
  size_t i;
  #pragma mta assert parallel
  for (i = 0; i < num; ++i) {
    writexf(&locks[i], val);
  }
}

inline
void init_locks_full(int *locks, const size_t num) {
  init_locks_full(locks, num, 0);
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
    order.push_back(i1);
    order.push_back(i2);
    order.push_back(i3);
  }
  Order(const dynamic_array<size_t> &order)
      : order(order), total_ordering(true) {
    // do nothing
  }
  Order(const dynamic_array<size_t> &order, const bool total)
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

struct Index {
  set<Triple, Order> stream[NUM_INDEX_STREAMS];
  int locks[NUM_INDEX_STREAMS];
  Index() {
    init_locks_full(this->locks, NUM_INDEX_STREAMS);
  }
  Index(const Order &order) {
    init_locks_full(this->locks, NUM_INDEX_STREAMS);
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_INDEX_STREAMS
    for (i = 0; i < NUM_INDEX_STREAMS; ++i) {
      set<Triple, Order> ordered (order);
      this->stream[i].swap(ordered);
    }
  }
  Index(Index &copy) {
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_INDEX_STREAMS
    for (i = 0; i < NUM_INDEX_STREAMS; ++i) {
      int x = readfe(&copy.locks[i]);
      writexf(&this->locks[i], x);
      this->stream[i] = copy.stream[i];
      writeef(&copy.locks[i], x);
    }
  }
  ~Index() {
    // do nothing
  }
  Index &operator=(Index &rhs) {
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_INDEX_STREAMS
    for (i = 0; i < NUM_INDEX_STREAMS; ++i) {
      int x = readfe(&rhs.locks[i]);
      this->stream[i] = rhs.stream[i];
      writeef(&rhs.locks[i], x);
      readfe(&this->locks[i]);
      writeef(&this->locks[i], x);
    }
    return *this;
  }
  void swap(Index &r) {
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_INDEX_STREAMS
    for (i = 0; i < NUM_INDEX_STREAMS; ++i) {
      readfe(&this->locks[i]);
      readfe(&r.locks[i]);
      this->stream[i].swap(r.stream[i]);
      std::swap(this->locks[i], r.locks[i]);
      writeef(&this->locks[i], this->locks[i]);
      writeef(&r.locks[i], r.locks[i]);
    }
  }
  bool insert(const Triple &t) {
    size_t h = hash_jenkins_one_at_a_time((const uint8_t *)&t, ((const uint8_t *)&t) + sizeof(Triple)) % NUM_INDEX_STREAMS;
    int x = readfe(&this->locks[h]);
    bool b = this->stream[h].insert(t).second;
    writeef(&this->locks[h], x);
    return b;
  }
  size_t erase(const Triple &t) {
    size_t h = hash_jenkins_one_at_a_time((const uint8_t *)&t, ((const uint8_t *)&t) + sizeof(Triple)) % NUM_INDEX_STREAMS;
    int x = readfe(&this->locks[h]);
    size_t c = this->stream[h].erase(t);
    writeef(&this->locks[h], x);
    return c;
  }
  size_t size() const {
    size_t sz = 0;
    size_t i;
    for (i = 0; i < NUM_INDEX_STREAMS; ++i) {
      sz += this->stream[i].size();
    }
    return sz;
  }
};


// GLOBAL DATA
Index idxspo (Order(0, 1, 2));
Index idxpos (Order(1, 2, 0));
Index idxosp (Order(2, 0, 1));

void load_data(const char *filename) {
  uint8_t buffer[DATA_SIZE];
  int64_t snap_error;
  snap_restore(filename, buffer, DATA_SIZE, &snap_error);
  size_t i;
  size_t max = DATA_SIZE / (3 * sizeof(constint_t));
  #pragma mta assert parallel
  #pragma mta max concurrency NUM_INDEX_STREAMS
  for (i = 0; i < max; ++i) {
    size_t bufoff = i * sizeof(constint_t) * 3;
    Triple triple;
    int j;
    #pragma mta assert parallel
    #pragma mta max concurrency 3
    for (j = 0; j < 3; ++j) {
      // XMT is big-endian, so no need to check.
      memcpy(&(triple.at[j]),
             &(buffer[bufoff + j*sizeof(constint_t)]),
             sizeof(constint_t));
#ifdef FAKE
      if (is_little_endian()) {
        reverse_bytes(triple.at[j]);
      }
#endif
    }
    DEBUG("load triple", hex);
    DEBUG("  subject = ", triple.at[0]);
    DEBUG("  predicate = ", triple.at[1]);
    DEBUG("  object = ", triple.at[2]);
    DEBUG("", dec);
    idxspo.insert(triple);
    idxpos.insert(triple);
    idxosp.insert(triple);
  }
}

void print_data(char *filename) {
  size_t numtriples = idxspo.size();
  constint_t *data = (constint_t*)malloc(3*numtriples*sizeof(constint_t));
  size_t i;
//  #pragma mta assert parallel
//  #pragma mta max concurrency NUM_INDEX_STREAMS
  #pragma mta loop serial
  for (i = 0; i < NUM_INDEX_STREAMS; ++i) {
    size_t offset = 0;
    size_t j;
    for (j = 0; j < i; ++j) {
      offset += idxspo.stream[j].size();
    }
    offset *= 3;
    set<Triple, Order>::const_iterator begin = idxspo.stream[i].begin();
    set<Triple, Order>::const_iterator end = idxspo.stream[i].end();
    set<Triple, Order>::const_iterator it;
    for (it = begin; it != end; ++it) {
//      #pragma mta assert parallel
//      #pragma mta max concurrency 3
      #pragma mta loop serial
      for (j = 0; j < 3; ++j) {
        data[offset+j] = it->at[j];
#ifdef FAKE
        if (is_little_endian()) {
          reverse_bytes(data[offset+j]);
        }
#endif
      }
      offset += 3;
    }
  }
  int64_t snap_error;
  snap_snapshot(filename, data, 3 * numtriples * sizeof(constint_t), &snap_error);
  free(data);
}



struct Relation {
  list<Tuple> stream[NUM_RELATION_STREAMS];
  int locks[NUM_RELATION_STREAMS];
  Relation() {
    init_locks_full(this->locks, NUM_RELATION_STREAMS);
  }
  Relation(Relation &copy) {
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_RELATION_STREAMS
    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
      int x = readfe(&copy.locks[i]);
      writexf(&this->locks[i], x);
      this->stream[i] = copy.stream[i];
      writeef(&copy.locks[i], x);
    }
  }
  ~Relation() {
    // do nothing
  }
  Relation &operator=(Relation &rhs) {
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_RELATION_STREAMS
    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
      int x = readfe(&rhs.locks[i]);
      this->stream[i] = rhs.stream[i];
      writeef(&rhs.locks[i], x);
      readfe(&this->locks[i]);
      writeef(&this->locks[i], x);
    }
    return *this;
  }
  void swap(Relation &r) {
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_RELATION_STREAMS
    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
      int x = readfe(&this->locks[i]);
      int y = readfe(&r.locks[i]);
      this->stream[i].swap(r.stream[i]);
      writeef(&this->locks[i], y);
      writeef(&r.locks[i], x);
    }
  }
  bool empty() {
    bool e = true;
    size_t i;
    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
      e &= this->stream[i].empty();
    }
    return e;
  }
  size_t size() {
    size_t sz = 0;
    size_t i;
    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
      sz += this->stream[i].size();
    }
    return sz;
  }
//  void hash_out(Relation &hashed_out, dynamic_array<size_t> &positions) {
//    Relation h;
//    size_t i;
//    #pragma mta assert parallel
//    #pragma mta max concurrency NUM_RELATION_STREAMS
//    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
//      Relation interim;
//      list<Tuple>::const_iterator it = hashed_out.stream[i].begin();
//      for (; it != hashed_out.stream[i].end(); ++it) {
//        // TODO here
//      }
//    }
//  }
  void even_out() {
    size_t sqmean = 0;
    size_t mean = 0;
    size_t i;
    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
      mean += this->stream[i].size();
      sqmean += this->stream[i].size() * this->stream[i].size();
    }
    mean /= NUM_RELATION_STREAMS;
    sqmean /= NUM_RELATION_STREAMS;
    size_t variance = sqmean - mean*mean;
    if ((float)variance / (float)mean < 0.1f) {
      return;
    }
    size_t stride;
    #pragma mta loop serial
    for (stride = 2; stride < 2*NUM_RELATION_STREAMS; stride *= 2) {
      #pragma mta assert parallel
      for (i = 0; i < NUM_RELATION_STREAMS; i += stride) {
        size_t j = i + (stride / 2);
        if (j < NUM_RELATION_STREAMS) {
          this->stream[i].splice(this->stream[i].end(), this->stream[j]);
        }
      }
    }
    #pragma mta loop serial
    for (stride /= 2; stride > 1; stride /= 2) {
      #pragma mta assert parallel
      for (i = 0; i < NUM_RELATION_STREAMS; i += stride) {
        size_t j = i + (stride / 2);
        if (j < NUM_RELATION_STREAMS) {
          size_t part = this->stream[i].size() *
                        min(stride / 2, (size_t)(NUM_RELATION_STREAMS - j)) /
                        min(stride    , (size_t)(NUM_RELATION_STREAMS    ));
          list<Tuple>::iterator it = this->stream[i].begin();
          size_t k;
          #pragma mta loop serial
          for (k = 0; k < part; ++k) {
            ++it;
          }
          this->stream[j].splice(this->stream[j].end(),
                                 this->stream[i],
                                 this->stream[i].begin(), it);
        }
      }
    }
  }
};

struct RelIndex {
  multiset<Tuple, Order> stream[NUM_RELATION_STREAMS];
  int locks[NUM_RELATION_STREAMS];
  RelIndex() {
    init_locks_full(this->locks, NUM_RELATION_STREAMS);
  }
  RelIndex(const Order &order) {
    init_locks_full(this->locks, NUM_RELATION_STREAMS);
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_RELATION_STREAMS
    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
      multiset<Tuple, Order> ordered (order);
      this->stream[i].swap(ordered);
    }
  }
  RelIndex(RelIndex &copy) {
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_RELATION_STREAMS
    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
      int x = readfe(&copy.locks[i]);
      writexf(&this->locks[i], x);
      this->stream[i] = copy.stream[i];
      writeef(&copy.locks[i], x);
    }
  }
  ~RelIndex() {
    // do nothing
  }
  RelIndex &operator=(RelIndex &rhs) {
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_RELATION_STREAMS
    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
      int x = readfe(&rhs.locks[i]);
      this->stream[i] = rhs.stream[i];
      writeef(&rhs.locks[i], x);
      readfe(&this->locks[i]);
      writeef(&this->locks[i], x);
    }
    return *this;
  }
  void swap(RelIndex &r) {
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_RELATION_STREAMS
    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
      readfe(&this->locks[i]);
      readfe(&r.locks[i]);
      this->stream[i].swap(r.stream[i]);
      std::swap(this->locks[i], r.locks[i]);
      writeef(&this->locks[i], this->locks[i]);
      writeef(&r.locks[i], r.locks[i]);
    }
  }
  void insert(const Tuple &t, dynamic_array<size_t> &positions) {
    uint32_t h = 0;
    size_t i;
    for (i = 0; i < positions.size(); ++i) {
      size_t j = positions[i];
      h = hash_jenkins_one_at_a_time_accum(h, (const uint8_t *)&t.at[j], (const uint8_t *)(&t.at[j] + 1));
    }
    h = hash_jenkins_one_at_a_time_end(h) % NUM_RELATION_STREAMS;
    int x = readfe(&this->locks[h]);
    this->stream[h].insert(t);
    writeef(&this->locks[h], x);
  }
  std::pair<multiset<Tuple, Order>::const_iterator, multiset<Tuple, Order>::const_iterator> equal_range(const Tuple &t, dynamic_array<size_t> &positions) {
    uint32_t h = 0;
    size_t i;
    for (i = 0; i < positions.size(); ++i) {
      size_t j = positions[i];
      h = hash_jenkins_one_at_a_time_accum(h, (const uint8_t *)&t.at[j], (const uint8_t *)(&t.at[j] + 1));
    }
    h = hash_jenkins_one_at_a_time_end(h) % NUM_RELATION_STREAMS;
    return this->stream[h].equal_range(t);
  }
  size_t size() const {
    size_t sz = 0;
    size_t i;
    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
      sz += this->stream[i].size();
    }
    return sz;
  }
};

struct RelUniqIndex {
  set<Tuple, Order> stream[NUM_RELATION_STREAMS];
  int locks[NUM_RELATION_STREAMS];
  RelUniqIndex() {
    init_locks_full(this->locks, NUM_RELATION_STREAMS);
  }
  RelUniqIndex(const Order &order) {
    init_locks_full(this->locks, NUM_RELATION_STREAMS);
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_RELATION_STREAMS
    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
      set<Tuple, Order> ordered (order);
      this->stream[i].swap(ordered);
    }
  }
  // not thread-safe
  RelUniqIndex(const RelUniqIndex &copy) {
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_RELATION_STREAMS
    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
      this->stream[i] = copy.stream[i];
      writexf(&this->locks[i], copy.locks[i]);
    }
  }
  ~RelUniqIndex() {
    // do nothing
  }
  RelUniqIndex &operator=(RelUniqIndex &rhs) {
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_RELATION_STREAMS
    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
      int x = readfe(&rhs.locks[i]);
      this->stream[i] = rhs.stream[i];
      writeef(&rhs.locks[i], x);
      readfe(&this->locks[i]);
      writeef(&this->locks[i], x);
    }
    return *this;
  }
  void swap(RelUniqIndex &r) {
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_RELATION_STREAMS
    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
      readfe(&this->locks[i]);
      readfe(&r.locks[i]);
      this->stream[i].swap(r.stream[i]);
      std::swap(this->locks[i], r.locks[i]);
      writeef(&this->locks[i], this->locks[i]);
      writeef(&r.locks[i], r.locks[i]);
    }
  }
  void insert(const Tuple &t) {
    uint32_t h = 0;
    size_t i;
    for (i = 0; i < TUPLE_SIZE; ++i) {
      h = hash_jenkins_one_at_a_time_accum(h, (const uint8_t *)&t.at[i], (const uint8_t *)(&t.at[i] + 1));
    }
    h = hash_jenkins_one_at_a_time_end(h) % NUM_RELATION_STREAMS;
    int x = readfe(&this->locks[h]);
    this->stream[h].insert(t);
    writeef(&this->locks[h], x);
  }
  size_t size() const {
    size_t sz = 0;
    size_t i;
    for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
      sz += this->stream[i].size();
    }
    return sz;
  }
};

typedef map<constint_t, RelUniqIndex> AtomMap;

// GLOBAL DATA
AtomMap atoms;

struct VarSet {
  set<varint_t> stream[NUM_VAR_SET_STREAMS];
  int locks[NUM_VAR_SET_STREAMS];
  VarSet() {
    init_locks_full(this->locks, NUM_VAR_SET_STREAMS);
  }
  VarSet(VarSet &copy) {
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_VAR_SET_STREAMS
    for (i = 0; i < NUM_VAR_SET_STREAMS; ++i) {
      int x = readfe(&copy.locks[i]);
      writexf(&this->locks[i], x);
      this->stream[i] = copy.stream[i];
      writeef(&copy.locks[i], x);
    }
  }
  ~VarSet() {
    // do nothing
  }
  VarSet &operator=(VarSet &rhs) {
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_VAR_SET_STREAMS
    for (i = 0; i < NUM_VAR_SET_STREAMS; ++i) {
      int x = readfe(&rhs.locks[i]);
      this->stream[i] = rhs.stream[i];
      writeef(&rhs.locks[i], x);
      readfe(&this->locks[i]);
      writeef(&this->locks[i], x);
    }
    return *this;
  }
  void swap(VarSet &r) {
    size_t i;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_VAR_SET_STREAMS
    for (i = 0; i < NUM_VAR_SET_STREAMS; ++i) {
      readfe(&this->locks[i]);
      readfe(&r.locks[i]);
      this->stream[i].swap(r.stream[i]);
      std::swap(this->locks[i], r.locks[i]);
      writeef(&this->locks[i], this->locks[i]);
      writeef(&r.locks[i], r.locks[i]);
    }
  }
  void insert(const varint_t &t) {
    // TODO need better hashing than this
    size_t h = t % NUM_VAR_SET_STREAMS;
    int x = readfe(&this->locks[h]);
    this->stream[h].insert(t);
    writeef(&this->locks[h], x);
  }
};

//inline
//bool need_locking(int rank, int nstreams, int nstructures) {
//  return nstreams < nstructures &&
//         ((unsigned int)(nstructures / nstreams) > 1 ||
//          rank < nstructures - nstreams);
//}

void minusrel(Relation &intermediate, Relation &negated, Relation &result) {
  cerr << "[ERROR] Negation of non-special formulas is currently unsupported." << endl;
}

#if 0
#undef DEBUG
#define DEBUG(a, b) cerr << a << b << endl
#endif
void join(Relation &lhs, Relation &rhs, dynamic_array<size_t> &vars, Relation &result) {
  DEBUG("Joining", "");
  DEBUG("  LHS size = ", lhs.size());
  DEBUG("  RHS size = ", rhs.size());
  lhs.even_out();
  rhs.even_out();
  Relation temp;
  Relation *l, *r;
  if (lhs.size() >= rhs.size()) {
    l = &lhs;
    r = &rhs;
  } else {
    l = &rhs;
    r = &lhs;
  }
  Order order(vars, false);
  RelIndex index (order);
  size_t i;
  #pragma mta assert parallel
  #pragma mta max concurrency NUM_RELATION_STREAMS
  for (i = 0; i < NUM_RELATION_STREAMS; ++i) {
    list<Tuple>::const_iterator it = r->stream[i].begin();
    list<Tuple>::const_iterator end = r->stream[i].end();
    for (; it != end; ++it) {
      index.insert(*it, vars);
    }
  }
  int rank;
  #pragma mta assert parallel
  #pragma mta max concurrency NUM_RELATION_STREAMS
  for (rank = 0; rank < NUM_RELATION_STREAMS; ++rank) {
    list<Tuple>::const_iterator it = l->stream[rank].begin();
    list<Tuple>::const_iterator end = l->stream[rank].end();
    #pragma mta loop serial
    for (; it != end; ++it) {
      std::pair<multiset<Tuple, Order>::const_iterator,
           multiset<Tuple, Order>::const_iterator> rng = index.equal_range(*it, vars);
      multiset<Tuple, Order>::const_iterator rit;
      #pragma mta loop serial
      for (rit = rng.first; rit != rng.second; ++rit) {
        Tuple r;
        #pragma mta assert parallel
        #pragma mta max concurrency TUPLE_SIZE
        for (i = 0; i < TUPLE_SIZE; ++i) {
          r.at[i] = max(it->at[i], rit->at[i]);
        }
        temp.stream[rank].push_back(r);
      }
    }
  }
  result.swap(temp);
  DEBUG("  Results size = ", result.size());
}
#if 0
#undef DEBUG
#define DEBUG(a, b)
#endif

bool special(Condition &condition, Relation &intermediate, Relation &filtered, const bool sign) {
  if (condition.type != ATOMIC) {
    return false;
  }
  switch (condition.get.atom.type) {
    case ATOM:
    case MEMBERSHIP:
    case SUBCLASS:
    case FRAME:
      return false;
    case EQUALITY:
    case EXTERNAL:
      cerr << "[ERROR] Haven't implemented handling of special equality and built-ins, yet." << endl;
      return false;
    default:
      cerr << "[ERROR] Unhandled case " << (int)condition.get.atom.type << " at line " << __LINE__ << endl;
      return false;
  }
}

inline
bool special(Condition &condition, Relation &intermediate, Relation &filtered) {
  return special(condition, intermediate, filtered, true);
}

// called only in a serial context
void query_atom(Atom &atom, VarSet &allvars, Relation &results) {
  // just doing a scan... great for XMT, bad for others.
  size_t i;
  size_t max = atom.arguments.end - atom.arguments.begin;
  Term *term = atom.arguments.begin;
  #pragma mta assert parallel
  #pragma mta max concurrency TUPLE_SIZE
  for (i = 0; i < max; ++i) {
    if (term[i].type == VARIABLE) {
      allvars.insert(term[i].get.variable);
    } else if (term[i].type != CONSTANT) {
      cerr << "[ERROR] Not handling lists or functions in atoms.  Results will be incorrect." << endl;
    }
  }
  RelUniqIndex &base = atoms[atom.predicate];
  Relation temp;
  int rank;
  #pragma mta assert parallel
  #pragma mta max concurrency NUM_RELATION_STREAMS
  for (rank = 0; rank < NUM_RELATION_STREAMS; ++rank) {
    set<Tuple, Order>::const_iterator begin = base.stream[rank].begin();
    set<Tuple, Order>::const_iterator end = base.stream[rank].end();
    set<Tuple, Order>::const_iterator tuple;

    Tuple result; init_tuple(result);
    #pragma mta loop serial
    for (tuple = begin; tuple != end; ++tuple) {
      Term *t = atom.arguments.begin;
      size_t max = atom.arguments.end - t;
      size_t i;
      for (i = 0; i < max; ++i) {
        if (t[i].type == VARIABLE) {
          result.at[t[i].get.variable] = tuple->at[i];
        } else if (term[i].type == CONSTANT) {
          if (tuple->at[i] != term[i].get.constant) {
            break;
          }
        }
      }
      if (i == max) {
        temp.stream[rank].push_back(result);
      }
    }
  }
  results.swap(temp);
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
  Relation intermediate;
  Term &subj = atom.get.frame.object;
  if (subj.type == LIST) {
    return;
  }
  if (subj.type == FUNCTION) {
    cerr << "[ERROR] Functions are currently unsupported." << endl;
    return;
  }
  std::pair<Term, Term> *slot = atom.get.frame.slots.begin;
  #pragma mta loop serial
  for (; slot != atom.get.frame.slots.end; ++slot) {
    //cerr << "Slot #" << (slot - atom.get.frame.slots.begin) << endl;
    Triple mintriple; init_triple(mintriple);
    Triple maxtriple; init_triple(maxtriple, CONSTINT_MAX);
    Term &pred = slot->first;
    Term &obj = slot->second;
    if (pred.type == LIST || obj.type == LIST) {
      Relation empty;
      results.swap(empty);
      return;
    }
    if (pred.type == FUNCTION || obj.type == FUNCTION) {
      cerr << "[ERROR] Function are not supported.  Results will be incorrect." << endl;
      return;
    }
    VarSet newvars;
    if (subj.type == CONSTANT) {
      mintriple.at[0] = maxtriple.at[0] = subj.get.constant;
    } else {
      newvars.insert(subj.get.variable);
    }
    if (pred.type == CONSTANT) {
      mintriple.at[1] = maxtriple.at[1] = pred.get.constant;
    } else {
      newvars.insert(pred.get.variable);
    }
    if (obj.type == CONSTANT) {
      mintriple.at[2] = maxtriple.at[2] = obj.get.constant;
    } else {
      newvars.insert(obj.get.variable);
    }
    DEBUG("triple match", hex);
    DEBUG("  subject ", mintriple.at[0]);
    DEBUG("  predicate ", mintriple.at[1]);
    DEBUG("  object ", mintriple.at[2]);
    DEBUG("", dec);
    int idx = (subj.type == CONSTANT ? 0x4 : 0x0) |
              (pred.type == CONSTANT ? 0x2 : 0x0) |
              ( obj.type == CONSTANT ? 0X1 : 0X0);
    Relation selection;
    //cerr << "Starting the loop" << endl;
    int rank;
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_INDEX_STREAMS
    for (rank = 0; rank < NUM_INDEX_STREAMS; ++rank) {
      set<Triple, Order>::const_iterator begin, it, end;
      switch (idx) {
        case 0x0: case 0x4: case 0x6: case 0x7:
          begin = idxspo.stream[rank].lower_bound(mintriple);
          end   = idxspo.stream[rank].upper_bound(maxtriple);
          break;
        case 0x2: case 0x3:
          begin = idxpos.stream[rank].lower_bound(mintriple);
          end   = idxpos.stream[rank].upper_bound(maxtriple);
          break;
        case 0x1: case 0x5:
          begin = idxosp.stream[rank].lower_bound(mintriple);
          end   = idxosp.stream[rank].upper_bound(maxtriple);
          break;
        default:
          cerr << "[ERROR] Unhandled case " << hex << idx << " at line " << dec << __LINE__ << endl;
          break;
      }
      for (it = begin; it != end; ++it) {
        Tuple result; init_tuple(result);
        if (subj.type == VARIABLE) {
          result.at[subj.get.variable] = it->at[0];
        }
        if (pred.type == VARIABLE) {
          result.at[pred.get.variable] = it->at[1];
        }
        if (obj.type == VARIABLE) {
          result.at[obj.get.variable] = it->at[2];
        }
        selection.stream[rank].push_back(result);
      }
    }
    //cerr << "After the loop" << endl;
    if (slot == atom.get.frame.slots.begin) {
      intermediate.swap(selection);
    } else {
      dynamic_array<size_t> joinvars;
      joinvars.reserve(TUPLE_SIZE);
      #pragma mta assert parallel
      #pragma mta max concurrency NUM_VAR_SET_STREAMS
      for (rank = 0; rank < NUM_VAR_SET_STREAMS; ++rank) {
        set<varint_t>::const_iterator it = allvars.stream[rank].begin();
        set<varint_t>::const_iterator end = allvars.stream[rank].end();
        for (; it != end; ++it) {
          if (newvars.stream[rank].count(*it) > 0) {
            joinvars.push_back(*it);
          }
        }
      }
      join(intermediate, selection, joinvars, intermediate);
    }
    DEBUG("After the join", "");
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_VAR_SET_STREAMS
    for (rank = 0; rank < NUM_VAR_SET_STREAMS; ++rank) {
      allvars.stream[rank].insert(newvars.stream[rank].begin(),
                                  newvars.stream[rank].end());
    }
    DEBUG("After aggregating the variables", "");
  }
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
          intermediate.swap(subresult);
          continue;
        }
        query(*subformula, newvars, subresult);
        if (subformula == condition.get.subformulas.begin) {
          intermediate.swap(subresult);
        } else {
          dynamic_array<size_t> joinvars;
          joinvars.reserve(TUPLE_SIZE);
          size_t rank;
          #pragma mta assert parallel
          #pragma mta max concurrency NUM_VAR_SET_STREAMS
          for (rank = 0; rank < NUM_VAR_SET_STREAMS; ++rank) {
            set<varint_t>::const_iterator it = allvars.stream[rank].begin();
            set<varint_t>::const_iterator end = allvars.stream[rank].end();
            for (; it != end; ++it) {
              if (newvars.stream[rank].count(*it) > 0) {
                joinvars.push_back(*it);
              }
            }
          }
          join(intermediate, subresult, joinvars, intermediate);
        }
        size_t rank;
        #pragma mta assert parallel
        #pragma mta max concurrency NUM_VAR_SET_STREAMS
        for (rank = 0; rank < NUM_VAR_SET_STREAMS; ++rank) {
          allvars.stream[rank].insert(newvars.stream[rank].begin(),
                                      newvars.stream[rank].end());
        }
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

void act(Atom &atom, Relation &results) {
  results.even_out();
  RelUniqIndex &index = atoms[atom.predicate];
  size_t max = atom.arguments.end - atom.arguments.begin;
  size_t rank;
  #pragma mta assert parallel
  #pragma mta max concurrency NUM_RELATION_STREAMS
  for (rank = 0; rank < NUM_RELATION_STREAMS; ++rank) {
    set<Tuple, Order>::const_iterator result = index.stream[rank].begin();
    set<Tuple, Order>::const_iterator end = index.stream[rank].end();
    for (; result != end; ++result) {
      Tuple tuple; init_tuple(tuple);
      size_t i;
      #pragma mta assert parallel
      #pragma mta max concurrency TUPLE_SIZE
      for (i = 0; i < max; ++i) {
        if (atom.arguments.begin[i].type == CONSTANT) {
          tuple.at[i] = atom.arguments.begin[i].get.constant;
        } else if (atom.arguments.begin[i].type == VARIABLE) {
          tuple.at[i] = result->at[atom.arguments.begin[i].get.variable];
        } else {
          cerr << "[ERROR] Lists and/or/function in target atom ar not currently support.  Results will be incorrect." << endl;
        }
      }
      index.insert(tuple);
    }
  }
}

void act(ActionBlock &action_block, Relation &results, Index &assertions, Index &retractions) {
  if (action_block.action_variables.begin != action_block.action_variables.end) {
    cerr << "[ERROR] Action variables are not supported." << endl;
    return;
  }
  Action *action = action_block.actions.begin;
  #pragma mta loop serial
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
          cerr << "[ERROR] For now, supporting only assertion/retraction of frames/triples/atoms." << endl;
          return;
        }
        Frame &frame = action->get.atom.get.frame;
        if (frame.object.type == LIST || frame.object.type == FUNCTION) {
          cerr << "[ERROR] Not supporting lists or functions in actions." << endl;
          return;
        }
        std::pair<Term, Term> *slot = frame.slots.begin;
        #pragma mta loop serial
        for (; slot != frame.slots.end; ++slot) {
          Triple triple; init_triple(triple);
          if (slot->first.type == LIST || slot->first.type == FUNCTION ||
              slot->second.type == LIST || slot->second.type == FUNCTION) {
            cerr << "[ERROR] Not supporting lists or functions in actions." << endl;
            return;
          }
          if (frame.object.type == CONSTANT) {
            triple.at[0] = frame.object.get.constant;
          }
          if (slot->first.type == CONSTANT) {
            triple.at[1] = slot->first.get.constant;
          }
          if (slot->second.type == CONSTANT) {
            triple.at[2] = slot->second.get.constant;
          }
          if (!results.empty() && frame.object.type == CONSTANT &&
              slot->first.type == CONSTANT && slot->second.type == CONSTANT) {
            if (action->type == ASSERT_FACT) {
              assertions.insert(triple);
            } else {
              retractions.insert(triple);
            }
            continue;
          }
          results.even_out();
          size_t rank;
          #pragma mta assert parallel
          #pragma mta max concurrency NUM_RELATION_STREAMS
          for (rank = 0; rank < NUM_RELATION_STREAMS; ++rank) {
            list<Tuple>::const_iterator tuple = results.stream[rank].begin();
            list<Tuple>::const_iterator end = results.stream[rank].end();
            for (; tuple != end; ++tuple) {
              Triple trip = triple;
              if (frame.object.type == VARIABLE) {
                trip.at[0] = tuple->at[frame.object.get.variable];
              }
              if (slot->first.type == VARIABLE) {
                trip.at[1] = tuple->at[slot->first.get.variable];
              }
              if (slot->second.type == VARIABLE) {
                trip.at[2] = tuple->at[slot->second.get.variable];
              }
              if (action->type == ASSERT_FACT) {
                assertions.insert(trip);
              } else {
                retractions.insert(trip);
              }
            }
          }
        }
        return;
      }
      default: {
        cerr << "[ERROR] Currently supporting only ASSERT_FACT and RETRACT_FACT actions." << endl;
        return;
      }
    }
  }
}

void infer(dynamic_array<Rule> &rules) {
  bool changed = true;
  map<constint_t, size_t> sizes;
  map<constint_t, RelUniqIndex>::const_iterator atomit = atoms.begin();
  #pragma mta loop serial
  for (; atomit != atoms.end(); ++atomit) {
    sizes[atomit->first] = atomit->second.size();
  }
  size_t ncycles = 0;
  #pragma mta loop serial
  while (changed) {
    cerr << "  Cycle " << ++ncycles << "..." << endl;
    changed = false;
    Index assertions (Order(0, 1, 2));
    Index retractions (Order(0, 1, 2));
    size_t rulecount;
    size_t nrules = rules.size();
    #pragma mta loop serial
    for (rulecount = 0; rulecount < nrules; ++rulecount) {
      cerr << "    Rule " << (rulecount+1) << "... ";
      Relation results;
      VarSet allvars;
      cerr << "querying... ";
      query(rules[rulecount].condition, allvars, results);
      cerr << results.size() << " results... acting... ";
      act(rules[rulecount].action_block, results, assertions, retractions);
      cerr << "accumulated total " << assertions.size() << " assertions and " << retractions.size() << " retractions." << endl;
    }
    int locks[NUM_INDEX_STREAMS];
    init_locks_full(locks, NUM_INDEX_STREAMS);
    size_t i;
    bool changes[NUM_INDEX_STREAMS];
    size_t nretractions[NUM_INDEX_STREAMS];
    size_t nassertions[NUM_INDEX_STREAMS];
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_INDEX_STREAMS
    for (i = 0; i < NUM_INDEX_STREAMS; ++i) {
      changes[i] = false;
      nretractions[i] = 0;
      nassertions[i] = 0;
    }
    size_t rank;
    FAKE_FOR_ALL_STREAMS(rank, nstreams, NUM_INDEX_STREAMS)
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_INDEX_STREAMS
    for (rank = 0; rank < NUM_INDEX_STREAMS; ++rank) {
      set<Triple, Order>::const_iterator it = retractions.stream[rank].begin();
      set<Triple, Order>::const_iterator end = retractions.stream[rank].end();
      for (; it != end; ++it) {
        if (idxspo.erase(*it) > 0) {
          idxpos.erase(*it);
          idxosp.erase(*it);
          changes[rank] = true;
          ++nretractions[rank];
        }
      }
    }
    size_t summation = 0;
    for (i = 0; i < NUM_INDEX_STREAMS; ++i) {
      summation += nretractions[i];
    }
    cerr << "  " << summation << " retractions, ";
    FAKE_FOR_ALL_STREAMS(rank, nstreams, NUM_INDEX_STREAMS)
    #pragma mta assert parallel
    #pragma mta max concurrency NUM_INDEX_STREAMS
    for (rank = 0; rank < NUM_INDEX_STREAMS; ++rank) {
      set<Triple, Order>::const_iterator it = assertions.stream[rank].begin();
      set<Triple, Order>::const_iterator end = assertions.stream[rank].end();
      for (; it != end; ++it) {
        if (idxspo.insert(*it)) {
          DEBUG("insert triple", hex);
          DEBUG("  subject = ", it->at[0]);
          DEBUG("  predicate = ", it->at[1]);
          DEBUG("  object = ", it->at[2]);
          DEBUG("", dec);
          idxpos.insert(*it);
          idxosp.insert(*it);
          changes[rank] = true;
          ++nassertions[rank];
        }
      }
    }
    summation = 0;
    for (i = 0; i < NUM_INDEX_STREAMS; ++i) {
      summation += nassertions[i];
      changed |= changes[i];
    }
    cerr << "  " << summation << " assertions." << endl;
    atomit = atoms.begin();
    #pragma mta loop serial
    for (; atomit != atoms.end(); ++atomit) {
      changed = changed || sizes[atomit->first] != atomit->second.size();
      sizes[atomit->first] = atomit->second.size();
    }
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
  cout << "loaded " << idxspo.size() << " triples." << endl;
  time_t time_start_inferring = time(NULL);
  cout << "Inferring at time " << time_start_inferring << "..." << endl;
  infer(rules);
  time_t time_start_writing_data = time(NULL);
  cout << "Writing data at time " << time_start_writing_data << "..." << endl;
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
