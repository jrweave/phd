#include <algorithm>
#include <cmath>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <mpi.h>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "main/encode.h"
#include "par/DistComputation.h"
#include "par/MPIPacketDistributor.h"
#include "ptr/DPtr.h"
#include "ptr/MPtr.h"
#include "util/timing.h"

bool RANDOMIZE = false;
bool COMPLETE = false;
int PAGESIZE = 4*1024*1024;
int NUMREQUESTS = 100;
int COORDEVERY = 100;
int PACKETSIZE = 128;

#ifndef NPROCS_PER_NODE
#define NPROCS_PER_NODE 1
#endif

#ifdef DEBUG
#undef DEBUG
#endif
//#define DEBUG(msg, val) cerr << "[DEBUG] " << __FILE__ << ':' << __LINE__ << ": " << (msg) << (val) << endl
#define DEBUG(msg, val)

using namespace par;
using namespace ptr;
using namespace std;

void *myalloc(size_t num_items, size_t item_size) {
#ifdef USE_POSIX_MEMALIGN
  void *p = NULL;
  size_t align = sizeof(void*);
  while (align < item_size) {
    align <<= 1;
  }
  if (posix_memalign(&p, align, num_items * item_size) != 0) {
    return NULL;
  }
  return p;
#else
  return malloc(num_items * item_size);
#endif
}

#define ZEROSAY(...) if (MPI::COMM_WORLD.Get_rank() == 0) cerr << __VA_ARGS__

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

uint8_t *read_all(const char *filename, size_t &len) {
  MPI::File file = MPI::File::Open(MPI::COMM_WORLD, filename,
      MPI::MODE_RDONLY, MPI::INFO_NULL);
  file.Seek(0, MPI_SEEK_SET);
  MPI::Offset filesize = file.Get_size();
  MPI::Offset bytesread = 0;
  uint8_t *buffer = (uint8_t*)myalloc(filesize, 1);
  while (bytesread < filesize) {
    MPI::Status stat;
    file.Read_all(buffer + bytesread, filesize - bytesread, MPI::BYTE, stat);
    bytesread += stat.Get_count(MPI::BYTE);
  }
  file.Close();
  len = (size_t) filesize;
  return buffer;
}

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

#ifndef USE_INDEX_SPO
#define USE_INDEX_SPO 0
#endif

#ifndef USE_INDEX_OSP
#define USE_INDEX_OSP 0
#endif

#ifndef TUPLE_SIZE
#if NPROCS_PER_NODE > 1
#error "When using -DNPROCS_PER_NODE=N, you must set -DTUPLE_SIZE=M where M is the maximum number of variables in any given rule."
#else
#warning "Without -DTUPLE_SIZE=N, tuples will be stored in vectors, which are inefficient.  If you know the maximum number of variables N in any rule, compile with -DTUPLE_SIZE=N."
#endif
#endif

#ifndef CONTAINER
#define CONTAINER deque
#elif CONTAINER == 0
#undef CONTAINER // Hacky way to force lists.
#endif

template<size_t N>
class Array {
private:
  constint_t data[N];
  size_t sz;
  void safety_check(size_t s) {
    if (s > N) {
      stringstream ss(stringstream::in | stringstream::out);
      ss << "[ERROR] You have requested size " << s << " but fixed to " << N << ".  You will probably see a segmentation fault or bus error soon hereafter." << endl;
      cerr << ss.str();
    }
  }
public:
  typedef constint_t* iterator;
  typedef const constint_t* const_iterator;
  Array() : sz(0) {
    fill_n(this->data, N, 0);
  }
  Array(size_t n)
      : sz(n) {
    safety_check(n);
    fill_n(this->data, N, 0);
  }
  Array(size_t n, const constint_t &val)
      : sz(n) {
    safety_check(n);
    fill_n(this->data, n, val);
    fill_n(this->data + n, N - n, 0);
  }
  Array(const Array<N> &copy)
      : sz(copy.sz) {
    std::copy(copy.data, copy.data + N, this->data);
  }
  ~Array() {}
  Array &operator=(const Array<N> &rhs) {
    this->sz = rhs.sz;
    copy(rhs.data, rhs.data + rhs.sz, this->data);
    return *this;
  }
  iterator begin() { return this->data; }
  const_iterator begin() const { return this->data; }
  iterator end() { return this->data + this->sz; }
  const_iterator end() const { return this->data + this->sz; }
  size_t size() const { return this->sz; }
  size_t max_size() const { return N; }
  void resize(size_t n) {
    safety_check(n);
    if (n < this->sz) {
      fill(this->data + n, this->data + this->sz, 0);
    }
    this->sz = n;
  }
  void resize(size_t n, constint_t val) {
    safety_check(n);
    if (this->sz < n) {
      fill(this->data + this->sz, this->data + n, val);
    } else if (n < this->sz) {
      fill(this->data + n, this->data + this->sz, 0);
    }
    this->sz = n;
  }
  size_t capacity() const { return N; }
  bool empty() const { return this->sz == 0; }
  void reserve(size_t  n) {
    safety_check(n);
  }
  constint_t &operator[](size_t i) { return this->data[i]; }
  const constint_t &operator[](size_t i) const { return this->data[i]; }
  constint_t &at(size_t i) { return this->data[i]; }
  const constint_t &at(size_t i) const { return this->data[i]; }
  constint_t &front() { return this->data[0]; }
  const constint_t &front() const { return this->data[0]; }
  constint_t &back() { return this->data[this->sz-1]; }
  const constint_t &back() const { return this->data[this->sz-1]; }
  void push_back(const constint_t &val) {
    safety_check(this->sz + 1);
    this->data[this->sz++] = val;
  }
  void pop_back(const constint_t &val) {
    this->data[--this->sz] = 0;
  }
  void swap(Array<N> &t) {
    size_t x = max(this->sz, t.sz);
    swap_ranges(this->data, this->data + x, t.data);
    std::swap(this->sz, t.sz);
  }
  void clear() {
    fill_n(this->data, this->sz, 0);
    this->sz = 0;
  }
  bool operator<(const Array<N> &t) const {
    return lexicographical_compare(this->data, this->data + this->sz,
                                   t.data, t.data + t.sz);
  }
};

typedef Array<3> Triple;

#ifdef TUPLE_SIZE
typedef Array<TUPLE_SIZE> Tuple;
#else
typedef vector<constint_t> Tuple;
#endif


#ifdef CONTAINER
typedef CONTAINER<Tuple> Relation;
#else
typedef list<Tuple> Relation;
#endif

class Order {
private:
  vector<size_t> order;
  bool total_ordering;
public:
  Order() : total_ordering(true) {
    // do nothing
  }
  Order(size_t i1, size_t i2, size_t i3) : total_ordering(false) {
    order.push_back(i1);
    order.push_back(i2);
    order.push_back(i3);
  }
  Order(const vector<size_t> &order)
      : order(order), total_ordering(true) {
    // do nothing
  }
  Order(const vector<size_t> &order, const bool total)
      : order(order), total_ordering(total) {
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
    return this->total_ordering ?
           t1 < t2 : false;
  }
  bool operator()(const Triple &t1, const Triple &t2) const {
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
    return this->total_ordering ?
           t1 < t2 : false;
  }
};

class Hash {
private:
  vector<size_t> order;
  bool total_ordering;
public:
  bool hack_randomize;
  Hash() : total_ordering(true), hack_randomize(false) {
    // do nothing
  }
  Hash(size_t i1, size_t i2, size_t i3) : total_ordering(false), hack_randomize(false) {
    order.push_back(i1);
    order.push_back(i2);
    order.push_back(i3);
  }
  Hash(const vector<size_t> &order)
      : order(order), total_ordering(true), hack_randomize(false) {
    // do nothing
  }
  Hash(const vector<size_t> &order, const bool total)
      : order(order), total_ordering(total), hack_randomize(false) {
    // do nothing
  }
  Hash(const Hash &copy)
      : order(copy.order), total_ordering(copy.total_ordering), hack_randomize(copy.hack_randomize) {}
  uint32_t operator()(const Tuple &t) const {
    if (this->hack_randomize) {
      return random();
    }
    size_t i;
    uint32_t h = 0;
    for (i = 0; i < this->order.size(); ++i) {
      size_t j;
      size_t p = order[i];
      for (j = 0; j < sizeof(constint_t); ++j) {
        uint8_t b = ((t[p] >> (j << 3)) & 0xFF);
        h += b;
        h += (h << 10);
        h ^= (h >> 6);
      }
    }
    if (this->total_ordering) {
      for (i = 0; i < t.size(); ++i) {
        size_t j;
        for (j = 0; j < sizeof(constint_t); ++j) {
          uint8_t b = ((t[i] >> (j << 3)) & 0xFF);
          h += b;
          h += (h << 10);
          h ^= (h >> 6);
        }
      }
    }
    h += (h << 3);
    h ^= (h >> 11);
    h += (h << 15);
    return h;
  }
  uint32_t operator()(const Triple &t) const {
    if (this->hack_randomize) {
      return random();
    }
    size_t i;
    uint32_t h = 0;
    for (i = 0; i < this->order.size(); ++i) {
      size_t j;
      size_t p = order[i];
      for (j = 0; j < sizeof(constint_t); ++j) {
        uint8_t b = ((t[p] >> (j << 3)) & 0xFF);
        h += b;
        h += (h << 10);
        h ^= (h >> 6);
      }
    }
    if (this->total_ordering) {
      for (i = 0; i < 3; ++i) {
        size_t j;
        for (j = 0; j < sizeof(constint_t); ++j) {
          uint8_t b = ((t[i] >> (j << 3)) & 0xFF);
          h += b;
          h += (h << 10);
          h ^= (h >> 6);
        }
      }
    }
    h += (h << 3);
    h ^= (h >> 11);
    h += (h << 15);
    return h;
  }
};

typedef set<Tuple, Order> Index;
typedef set<Triple, Order> TripleIndex;
TripleIndex idxspo (Order(0, 1, 2));
TripleIndex idxpos (Order(1, 2, 0));
TripleIndex idxosp (Order(2, 0, 1));
map<constint_t, Index> atoms;

MPI::Intracomm COMM_LOCAL;
MPI::Intracomm COMM_REPLS;

void load_data(const char *filename) {
//  ifstream fin(filename);
  int rank = MPI::COMM_WORLD.Get_rank();
  int commsize = MPI::COMM_WORLD.Get_size();
  string fnamestr(filename);
  size_t hash = fnamestr.find('#');
  if (hash == string::npos && commsize > 1) {
    if (rank == 0) {
      cerr << "[ERROR] File name must contain a '#' to be replaced with processor rank." << endl;
    }
    return;
  }
  if (hash != string::npos) {
    stringstream ss (stringstream::in | stringstream::out);
    ss << fnamestr.substr(0, hash) << rank << fnamestr.substr(hash + 1, fnamestr.size() - hash - 1);
    fnamestr = ss.str();
  }
  MPI::File file = MPI::File::Open(MPI::COMM_SELF, fnamestr.c_str(),
      MPI::MODE_RDONLY, MPI::INFO_NULL);
  file.Seek(0, MPI_SEEK_SET);
  MPI::Offset filesize = file.Get_size();
  if (filesize <= 0) {
    file.Close();
    return;
  }
  uint8_t *buffer = (uint8_t*)myalloc(1, PAGESIZE);
  uint8_t *data = (uint8_t*)myalloc(1, PAGESIZE);
  MPI::Offset bytesread = 0;
  MPI::Offset dangling = 0;
  MPI::Offset unitsize = 3 * sizeof(constint_t);
  MPI::Offset amount = min(filesize, (MPI::Offset)PAGESIZE);
  MPI::Request req = file.Iread(buffer, amount, MPI::BYTE);
  while (bytesread < filesize) {
    MPI::Status stat;
    while (!req.Test(stat)) {
      // spin until reading is complete
    }
    swap(data, buffer);
    MPI::Offset nbytes = stat.Get_count(MPI::BYTE);
    bytesread += nbytes;
    MPI::Offset datalen = nbytes + dangling;
    dangling = datalen % unitsize;
    datalen -= dangling;
    memcpy(buffer, data + datalen, dangling);
    if (bytesread < filesize) {
      amount = min(filesize - bytesread, ((MPI::Offset)PAGESIZE) - dangling);
      req = file.Iread(buffer + dangling, amount, MPI::BYTE);
    }
    const uint8_t *p = data;
    const uint8_t *end = data + datalen;
    while(p != end) {
      Triple triple(3);
      Triple::iterator it = triple.begin();
      for (; it != triple.end(); ++it) {
        *it = 0;
        const uint8_t *e = p + sizeof(constint_t);
        for (; p != e; ++p) {
          *it = (*it << 8) | ((constint_t)*p);
        }
      }
      idxpos.insert(triple);
    }
  }
  file.Close();
  free(data);
  free(buffer);
}

void minusrel(Relation &intermediate, Relation &negated, Relation &result) {
  cerr << "[ERROR] Negation on non-special formulas is currently unsupported." << endl;
}

bool join(Tuple &t1, const Tuple &t2, Tuple &r) {
  if (t1.size() < t2.size()) {
    t1.resize(t2.size());
  }
  Tuple temp(t1.size());
  int i;
  for (i = 0; i < t1.size(); ++i) {
    if (i >= t2.size() || t2[i] == 0) {
      temp[i] = t1[i];
    } else if (t1[i] == 0 || t1[i] == t2[i]) {
      temp[i] = t2[i];
    } else {
      return false;
    }
  }
  r.swap(temp);
  return true;
}

class HashTuples : public DistComputation {
private:
  Hash hash;
  Relation::const_iterator begin, end;
  int rank, nproc;
public:
  Relation hashed;
  HashTuples(Distributor *dist, int rank, int np, const Relation &rel, Hash h) throw(BaseException<void*>)
      : DistComputation(dist), rank(rank), nproc(np), begin(rel.begin()), end(rel.end()), hash(h) {
    // do nothing
  }
  virtual ~HashTuples() throw(DistException) {
    // do nothing
  }
  void start() throw(TraceableException) {}
  void finish() throw(TraceableException) {}
  void fail() throw() {
    cerr << "[ERROR] Processor " << MPI::COMM_WORLD.Get_rank() << " experienced a failure when attempting to hash distribute tuples." << endl;
  }
  int pickup(DPtr<uint8_t> *&buffer, size_t &len) 
      throw(BadAllocException, TraceableException) {
    if (this->begin == this->end) {
      return -2;
    }
    int send_to = this->hash(*this->begin) % this->nproc;
    if (send_to == this->rank) {
      this->hashed.push_back(*this->begin);
      return (++this->begin == this->end ? -2 : -1);
    }
    len = TUPLE_SIZE * sizeof(constint_t);
    if (buffer->size() < len) {
      buffer->drop();
      try {
        NEW(buffer, MPtr<uint8_t>, len);
      } RETHROW_BAD_ALLOC
    }
    memcpy(buffer->dptr(), this->begin->begin(), len);
    ++this->begin;
    return send_to;
  }
  void dropoff(DPtr<uint8_t> *msg) throw(TraceableException) {
    if (msg->size() != TUPLE_SIZE*sizeof(constint_t)) {
      THROW(TraceableException, "Failed sanity check.");
    }
    Tuple t(TUPLE_SIZE);
    memcpy(t.begin(), msg->dptr(), TUPLE_SIZE*sizeof(constint_t));
    this->hashed.push_back(t);
  }
};

// Set to 1 to use sort-merge join instead of one-sided hash join
#if 0
void join(Relation &lhs, Relation &rhs, vector<size_t> &vars, Relation &result) {
  DEBUG("Joining", "");
  DEBUG("  LHS size = ", lhs.size());
  DEBUG("  RHS size = ", rhs.size());
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
  DEBUG("  Result size = ", result.size());
}
#else
void join(Relation &lhs, Relation &rhs, vector<size_t> &vars, Relation &result) {
#if NPROCS_PER_NODE > 1
  int local_rank = COMM_LOCAL.Get_rank();
  int local_nproc = COMM_LOCAL.Get_size();
  Hash hash(vars, false);
  Distributor *dist;
  HashTuples *hash_tuples;
  NEW(dist, MPIPacketDistributor, COMM_LOCAL, TUPLE_SIZE*sizeof(constint_t),
      NUMREQUESTS, COORDEVERY, 789);
  NEW(hash_tuples, HashTuples, dist, local_rank, local_nproc, lhs, hash);
  hash_tuples->exec();
  lhs.swap(hash_tuples->hashed);
  DELETE(hash_tuples);
  NEW(dist, MPIPacketDistributor, COMM_LOCAL, TUPLE_SIZE*sizeof(constint_t),
      NUMREQUESTS, COORDEVERY, 790);
  NEW(hash_tuples, HashTuples, dist, local_rank, local_nproc, rhs, hash);
  hash_tuples->exec();
  rhs.swap(hash_tuples->hashed);
  DELETE(hash_tuples);
#endif
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
  multiset<Tuple, Order> index (order);
  Relation::iterator it = r->begin();
  for (; it != r->end(); ++it) {
    index.insert(*it);
  }
  for (it = l->begin(); it != l->end(); ++it) {
    pair<multiset<Tuple, Order>::iterator,
         multiset<Tuple, Order>::iterator> rng = index.equal_range(*it);
    multiset<Tuple, Order>::iterator rit;
    for (rit = rng.first; rit != rng.second; ++rit) {
      Tuple res;
      if (!join(*it, *rit, res)) {
        cerr << "[ERROR] Two tuples that were expected to be compatible turned out not to be.  This indicates a flaw in the program logic." << endl;
      }
      temp.push_back(res);
    }    
  }
  result.swap(temp);
}
#endif

bool special(Condition &condition, Relation &intermediate, Relation &filtered, bool sign) {
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
        if (( sign && lhs->get.constant != rhs->get.constant) ||
            (!sign && lhs->get.constant == rhs->get.constant)) {
          Relation empty;
          filtered.swap(empty);
        } else {
          filtered = intermediate;
        }
      } else if (lhs->type == CONSTANT) {
        Relation result;
        Relation::iterator it = intermediate.begin();
        for (; it != intermediate.end(); ++it) {
          if (it->size() <= rhs->get.variable) {
            it->resize(rhs->get.variable + 1);
          }
          if (it->at(rhs->get.variable) == 0) {
            if (sign) {
              result.push_back(*it);
              Tuple &t = result.back();
              t[rhs->get.variable] = lhs->get.constant;
            } else {
              cerr << "[ERROR] Variables in inequality should be bound by atomic formulas on the left." << endl;
            }
          } else if (( sign && it->at(rhs->get.variable) == lhs->get.constant) ||
                     (!sign && it->at(rhs->get.variable) != lhs->get.constant)) {
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
            cerr << "[WARNING] Please make sure equality statements occur to the right of atomic formulas that bind a variable in the equality, if any exist.  Otherwise, results will be incorrect." << endl;
            return false;
          } else if (!sign && (c1 == 0 || c2 == 0)) {
            cerr << "[ERROR] Please make sure that inequality statements occur to the right of atomic formulas that bind BOTH variables and the inequality." << endl;
            return false;
          } else if (c1 == 0) {
            result.push_back(*it);
            Tuple &t = result.back();
            t[lhs->get.variable] = c2;
          } else if (c2 == 0) {
            result.push_back(*it);
            Tuple &t = result.back();
            t[rhs->get.variable] = c1;
          } else if (sign && c1 == c2) {
            result.push_back(*it);
          } else if (!sign && c1 != c2) {
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
      Relation results;
      Relation::iterator it = intermediate.begin();
      for (; it != intermediate.end(); ++it) {
        if (it->size() <= arg2->get.variable) {
          it->resize(arg2->get.variable + 1);
        }
        constint_t c = it->at(arg2->get.variable);
        if (c == 0) {
          cerr << "[ERROR] Variables in built-ins must already be bound by atomic formulas to the left." << endl;
//          Term *t = arg1->get.termlist.begin;
//          for (; t != arg1->get.termlist.end; ++t) {
//            if (t->type == CONSTANT) {
//              results.push_back(*it);
//              results.back().at(arg2->get.variable) = t->get.constant;
//            }
//          }
        } else {
          Term *t = arg1->get.termlist.begin;
          for (; t != arg1->get.termlist.end; ++t) {
            if (t->type == CONSTANT && t->get.constant == c) {
              if (sign) {
                results.push_back(*it);
              }
              break;
            }
          }
          if (!sign && t == arg1->get.termlist.end) {
            results.push_back(*it);
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

bool special(Condition &condition, Relation &intermediate, Relation &filtered) {
  return special(condition, intermediate, filtered, true);
}

void query_atom(Atom &atom, set<varint_t> &allvars, Relation &results) {
  // just doing a scan
  // in the general case, this is terribly inefficient
  // but since I will need it only for dealing with rdf:Lists
  // which are presumably quite short, it should be okay
  // ... hmmm... not so sure about this TODO
  set<varint_t> newvars;
  varint_t maxvar = 0;
  size_t numvars = 0;
  Term *term = atom.arguments.begin;
  for (; term != atom.arguments.end; ++term) {
    if (term->type == VARIABLE) {
      newvars.insert(term->get.variable);
      maxvar = max(maxvar, term->get.variable);
      ++numvars;
    } else if (term->type != CONSTANT) {
      cerr << "[ERROR] Not handling lists or functions in atoms." << endl;
      return;
    }
  }
  if (numvars > newvars.size()) {
    cerr << "[ERROR] Results are incorrect when the same variable occurs multiple times in an atom in the rule body." << endl;
    return;
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

void scan_triples(Triple &triple_pattern, TripleIndex &results) {
  TripleIndex::const_iterator it = idxpos.begin();
  for (; it != idxpos.end(); ++it) {
    size_t i;
    for (i = 0; i < 3; ++i) {
      if (triple_pattern[i] != 0 && triple_pattern[i] != it->at(i)) {
        break;
      }    
    }    
    if (i == 3) { 
      results.insert(*it);
    }    
  }
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
  Triple mintriple(3);
  Triple maxtriple(3);
  if (subj.type == CONSTANT) {
    mintriple[0] = maxtriple[0] = subj.get.constant;
  } else {
    mintriple[0] = 0;
    maxtriple[0] = CONSTINT_MAX;
    maxvar = subj.get.variable;
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
    TripleIndex::const_iterator begin, end;
    TripleIndex scanned (Order(1,2,0));
    switch (idx) {
      case 0x4:
      case 0x6: // SPO
#if USE_INDEX_SPO
        begin = idxspo.lower_bound(mintriple);
        end = idxspo.upper_bound(maxtriple);
#else
        scan_triples(mintriple, scanned);
        begin = scanned.begin();
        end = scanned.end();
#endif
        break;
      case 0x0:
        begin = idxpos.begin();
        end = idxpos.end();
        break;
      case 0x2:
      case 0x3:
      case 0x7: // POS
        begin = idxpos.lower_bound(mintriple);
        end = idxpos.upper_bound(maxtriple);
        break;
      case 0x1:
      case 0x5: // OSP
#if USE_INDEX_OSP
        begin = idxosp.lower_bound(mintriple);
        end = idxosp.upper_bound(maxtriple);
#else
        scan_triples(mintriple, scanned);
        begin = scanned.begin();
        end = scanned.end();
#endif
        break;
      default:
        cerr << "[ERROR] Unhandled case " << hex << idx << " at line " << dec << __LINE__ << endl;
        return;
    }
    // TODO the following loop could probably be more efficient
    Relation selection;
    for (; begin != end; ++begin) {
      Tuple result(maxvar + 1);
      if (subj.type == VARIABLE) {
        result[subj.get.variable] = begin->at(0);
      }
      if (pred.type == VARIABLE) {
        if (subj.type == VARIABLE && subj.get.variable == pred.get.variable) {
          if (begin->at(0) != begin->at(1)) {
            continue;
          }
        }
        result[pred.get.variable] = begin->at(1);
      }
      if (obj.type == VARIABLE) {
        if (subj.type == VARIABLE && subj.get.variable == obj.get.variable) {
          if (begin->at(0) != begin->at(2)) {
            continue;
          }
        }
        if (pred.type == VARIABLE && pred.get.variable == obj.get.variable) {
          if (begin->at(1) != begin->at(2)) {
            continue;
          }
        }
        result[obj.get.variable] = begin->at(2);
      }
      selection.push_back(result);
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
#ifdef CONTAINER
        intermediate.insert(intermediate.end(), subresult.begin(), subresult.end());
#else
        intermediate.splice(intermediate.end(), subresult);
#endif
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

#if 1 /* FAST WAY */

    if (special(*cond, intermediate, negresult, false)) {
      intermediate.swap(negresult);
      continue;
    }
    
#else /* SLOW WAY */

    if (special(*cond, intermediate, negresult)) {
#ifdef CONTAINER
      sort(intermediate.begin(), intermediate.end());
      sort(negresult.begin(), negresult.end());
#else
      intermediate.sort();
      negresult.sort();
#endif
      Relation leftover(intermediate.size());
      Relation::iterator iit = set_difference(intermediate.begin(),
          intermediate.end(), negresult.begin(), negresult.end(),
          leftover.begin());
      Relation newinter;
#ifdef CONTAINER
      newinter.insert(newinter.end(), leftover.begin(), iit);
#else
      newinter.splice(newinter.end(), leftover,
                      leftover.begin(), iit);
#endif
      intermediate.swap(newinter);
      continue;
    }

#endif /* END WAYS */

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

void act(ActionBlock &action_block, Relation &results, TripleIndex &assertions, TripleIndex &retractions) {
  if (action_block.action_variables.begin != action_block.action_variables.end) {
    cerr << "[ERROR] Action variables are unsupported." << endl;
    return;
  }
#if NPROCS_PER_NODE > 1
  int local_rank = COMM_LOCAL.Get_rank();
  int local_nproc = COMM_LOCAL.Get_size();
  Hash hash;
  hash.hack_randomize = true;
  Distributor *dist;
  HashTuples *hash_tuples;
  NEW(dist, MPIPacketDistributor, COMM_LOCAL, TUPLE_SIZE*sizeof(constint_t),
      NUMREQUESTS, COORDEVERY, 833);
  NEW(hash_tuples, HashTuples, dist, local_rank, local_nproc, results, hash);
  hash_tuples->exec();
  results.swap(hash_tuples->hashed);
  DELETE(hash_tuples);
#endif
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
        Triple triple(3);
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

void infer(Rule &rule, TripleIndex &assertions, TripleIndex &retractions) {
  Relation results;
  set<varint_t> allvars;
  query(rule.condition, allvars, results);
  act(rule.action_block, results, assertions, retractions);
}

#define USE_OLD_WAY 0
#if !USE_OLD_WAY
void note_sizes(vector<size_t> &sizes) {
  vector<size_t> sz;
  sz.reserve(1 + atoms.size());
  sz.push_back(idxpos.size());
  map<constint_t, Index>::const_iterator it = atoms.begin();
  for (; it != atoms.end(); ++it) {
    sz.push_back(it->second.size());
  }
  sizes.swap(sz);
}

void infer(vector<Rule> &rules) {
  size_t rules_since_change = 0;
  size_t cycle_count;
  vector<size_t> old_sizes;
  note_sizes(old_sizes);
  for (cycle_count = 0; rules_since_change < rules.size(); ++cycle_count) {
    size_t rulecount;
    for (rulecount = 0; rulecount < rules.size() && rules_since_change < rules.size(); ++rulecount) {
      bool changed = true;
      size_t app_count;
      for (app_count = 0; changed; ++app_count) {
        changed = false;

        TripleIndex assertions (Order(1, 2, 0));
        TripleIndex retractions (Order(1, 2, 0));
        infer(rules[rulecount], assertions, retractions);

        TripleIndex::iterator it = retractions.begin();
        for (; it != retractions.end(); ++it) {
          if (idxpos.erase(*it) > 0) {
#if USE_INDEX_SPO
            idxspo.erase(*it);
#endif
#if USE_INDEX_OSP
            idxosp.erase(*it);
#endif
            changed = true;
          }
        }
        it = assertions.begin();
        for (; it != assertions.end(); ++it) {
          if (idxpos.insert(*it).second) {
#if USE_INDEX_SPO
            idxspo.insert(*it);
#endif
#if USE_INDEX_OSP
            idxosp.insert(*it);
#endif
            changed = true;
          }
        }

        vector<size_t> new_sizes;
        note_sizes(new_sizes);
        bool self_change = changed || old_sizes != new_sizes;
        old_sizes.swap(new_sizes);
        COMM_LOCAL.Allreduce(&self_change, &changed, 1, MPI::BOOL, MPI::LOR);
      }

      if (app_count > 1) {
        rules_since_change = 1;
      } else {
        ++rules_since_change;
      }
    }
  }
  int rank = MPI::COMM_WORLD.Get_rank();
  int inconsistent = atoms[CONST_RIF_ERROR].empty() ? 0 : 1; 
  if (inconsistent > 0) { 
    stringstream ss(stringstream::in | stringstream::out);
    ss << "Processor " << rank << " is inconsistent." << endl;
    cerr << ss.str() << flush;
  }
  int nproc_inconsistent;
  MPI::COMM_WORLD.Reduce(&inconsistent, &nproc_inconsistent, 1, MPI::INT,
                         MPI::SUM, 0);
  if (rank == 0 && nproc_inconsistent > 0) { 
    cerr << nproc_inconsistent << " processors were inconsistent." << endl;
    cerr << "INCONSISTENT" << endl;  // MUST HAVE THIS LAST!
  }
}
#else
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
#ifndef ANY_ORDER
    TripleIndex assertions (Order(1, 2, 0));
    TripleIndex retractions (Order(1, 2, 0));
#endif
    int rulecount = 0;
    vector<Rule>::iterator rule = rules.begin();
    for (; rule != rules.end(); ++rule) {
#ifdef ANY_ORDER
      TripleIndex assertions (Order(1, 2, 0));
      TripleIndex retractions (Order(1, 2, 0));
#endif
      infer(*rule, assertions, retractions);
#ifndef ANY_ORDER
    }
#endif
    TripleIndex::iterator it = retractions.begin();
    for (; it != retractions.end(); ++it) {
      if (idxpos.erase(*it) > 0) {
#if USE_INDEX_SPO
        idxspo.erase(*it);
#endif
#if USE_INDEX_OSP
        idxosp.erase(*it);
#endif
        changed = true;
      }
    }
    it = assertions.begin();
    for (; it != assertions.end(); ++it) {
      if (idxpos.insert(*it).second) {
#if USE_INDEX_SPO
        idxspo.insert(*it);
#endif
#if USE_INDEX_OSP
        idxosp.insert(*it);
#endif
        changed = true;
      }
    }
#ifdef ANY_ORDER
    }
#endif
    atomit = atoms.begin();
    for (; atomit != atoms.end(); ++atomit) {
      changed = changed || sizes[atomit->first] != atomit->second.size();
      sizes[atomit->first] = atomit->second.size();
    }
  }
  int rank = MPI::COMM_WORLD.Get_rank();
  int inconsistent = atoms[CONST_RIF_ERROR].empty() ? 0 : 1; 
  if (inconsistent > 0) { 
    stringstream ss(stringstream::in | stringstream::out);
    ss << "Processor " << rank << " is inconsistent." << endl;
    cerr << ss.str() << flush;
  }
  int nproc_inconsistent;
  MPI::COMM_WORLD.Reduce(&inconsistent, &nproc_inconsistent, 1, MPI::INT,
                         MPI::SUM, 0);
  if (rank == 0 && nproc_inconsistent > 0) { 
    cerr << nproc_inconsistent << " processors were inconsistent." << endl;
    cerr << "INCONSISTENT" << endl;  // MUST HAVE THIS LAST!
  }
}
#endif




///// IO AND DISTRIBUTION /////

class DistUniq : public DistComputation {
private:
  int rank, nproc;
  TripleIndex::const_iterator it;
  TripleIndex::const_iterator end;
public:
  TripleIndex newindex;
  DistUniq(Distributor *dist) throw(BaseException<void*>)
      : DistComputation(dist), rank(MPI::COMM_WORLD.Get_rank()),
        nproc(MPI::COMM_WORLD.Get_size()), it(idxpos.begin()),
        end(idxpos.end()),
        newindex(TripleIndex(Order(1,2,0), allocator<Triple>())) {
    // do nothing
  }
  virtual ~DistUniq() throw(DistException) {
    // do nothing
  }
  void start() throw(TraceableException) {}
  void finish() throw(TraceableException) {}
  void fail() throw() {
    cerr << "[ERROR] Processor " << this->rank << " experienced a failure when attempting to redistributed the data in preparation for decoding." << endl;
  }
  int pickup(DPtr<uint8_t> *&buffer, size_t &len)
      throw(BadAllocException, TraceableException) {
    if (this->it == this->end) {
      return -2;
    }
    uint32_t send_to = 0;
    size_t i;
    const Triple &tuple = *this->it;
    ++this->it;

#if 1
    // hacking hash_jenkins_one_at_a_time into here for better distribution
    send_to = 0;
    for (i = 0; i < 3; ++i) {
      size_t j;
      for (j = 0; j < sizeof(constint_t); ++j) {
        uint8_t b = ((tuple[i] >> (j << 3)) & 0xFF);
        send_to += b;
        send_to += (send_to << 10);
        send_to ^= (send_to >> 6);
      }
    }
    send_to += (send_to << 3);
    send_to ^= (send_to >> 11);
    send_to += (send_to << 15);
    send_to %= this->nproc;
#else
    for (i = 0; i < sizeof(uint32_t); ++i) {
      send_to = (send_to << 8) | ((tuple[0] >> ((sizeof(constint_t)-i-1) << 3)) & 0xFF);
    }
    if (send_to >= this->nproc) {
      // This can happen for replicated encodings with most sig bit set to 1.
      send_to = 0;
      for (i = 0; i < sizeof(uint32_t); ++i) {
        send_to = (send_to << 8) | ((tuple[0] >> ((sizeof(uint32_t)-i-1) << 3)) & 0xFF);
      }
      send_to = send_to % this->nproc;
    }
#endif

    if (this->rank == send_to) {
      this->newindex.insert(tuple);
      return -1;
    }
    len = (sizeof(constint_t) << 1) + sizeof(constint_t); // 3*sizeof(constint_t)
    if (buffer->size() < len) {
      buffer->drop();
      try {
        NEW(buffer, MPtr<uint8_t>, len);
      } RETHROW_BAD_ALLOC
    }
    uint8_t *write_to = buffer->dptr();
    memcpy(write_to, &tuple[0], sizeof(constint_t));
    write_to += sizeof(constint_t);
    memcpy(write_to, &tuple[1], sizeof(constint_t));
    write_to += sizeof(constint_t);
    memcpy(write_to, &tuple[2], sizeof(constint_t));
    return (int)send_to;
  }
  void dropoff(DPtr<uint8_t> *msg) throw(TraceableException) {
    Triple triple(3);
    const uint8_t *read_from = msg->dptr();
    memcpy(&triple[0], read_from, sizeof(constint_t));
    read_from += sizeof(constint_t);
    memcpy(&triple[1], read_from, sizeof(constint_t));
    read_from += sizeof(constint_t);
    memcpy(&triple[2], read_from, sizeof(constint_t));
    this->newindex.insert(triple);
  }
};

class Redistributor : public DistComputation {
private:
  int rank, nproc, repl_count;
  TripleIndex::const_iterator it;
  TripleIndex::const_iterator end;
public:
  TripleIndex newindex;
  TripleIndex replications;
  bool randomize;
  Redistributor(Distributor *dist) throw(BaseException<void*>)
      : DistComputation(dist), rank(MPI::COMM_WORLD.Get_rank()),
        nproc(MPI::COMM_WORLD.Get_size()), it(idxpos.begin()),
        end(idxpos.end()), repl_count(0), randomize(false),
        newindex(TripleIndex(Order(1,2,0), allocator<Triple>())),
        replications(TripleIndex(Order(1,2,0), allocator<Triple>())) {
    // do nothing
  }
  Redistributor(Distributor *dist, int rank, int np, TripleIndex &trips) throw(BaseException<void*>)
      : DistComputation(dist), rank(rank), nproc(np),
        it(trips.begin()),
        end(trips.end()), repl_count(0), randomize(false),
        newindex(TripleIndex(Order(1,2,0), allocator<Triple>())),
        replications(TripleIndex(Order(1,2,0), allocator<Triple>())) {
    // do nothing
  }
  virtual ~Redistributor() throw(DistException) {
    // do nothing
  }
  void start() throw(TraceableException) {
    if (!this->randomize) {
      this->it = this->replications.begin();
      this->end = this->replications.end();
    }
  }
  void finish() throw(TraceableException) {}
  void fail() throw() {
    cerr << "[ERROR] Processor " << this->rank << " experienced a failure when attempting to redistribute the data." << endl;
  }
  int pickup(DPtr<uint8_t> *&buffer, size_t &len) 
      throw(BadAllocException, TraceableException) {
    if (this->it == this->end) {
      return -2;
    }
    len = (sizeof(constint_t) << 1) + sizeof(constint_t); // 3*sizeof(constint_t)
    if (buffer->size() < len) {
      buffer->drop();
      try {
        NEW(buffer, MPtr<uint8_t>, len);
      } RETHROW_BAD_ALLOC
    }
    uint8_t *write_to = buffer->dptr();
    memcpy(write_to, &(it->at(0)), sizeof(constint_t));
    write_to += sizeof(constint_t);
    memcpy(write_to, &(it->at(1)), sizeof(constint_t));
    write_to += sizeof(constint_t);
    memcpy(write_to, &(it->at(2)), sizeof(constint_t));
    int send_to;
    if (this->randomize && this->repl_count <= 0 &&
        this->replications.count(*(this->it)) <= 0) {
      send_to = random() % this->nproc;
      ++this->it;
    } else {
      send_to = (this->rank + this->repl_count) % this->nproc;
      ++this->repl_count;
      if (this->repl_count >= this->nproc) {
        ++this->it;
        this->repl_count = 0;
      }
    }
    return send_to;
  }
  void dropoff(DPtr<uint8_t> *msg) throw(TraceableException) {
    Triple triple(3);
    const uint8_t *read_from = msg->dptr();
    memcpy(&triple[0], read_from, sizeof(constint_t));
    read_from += sizeof(constint_t);
    memcpy(&triple[1], read_from, sizeof(constint_t));
    read_from += sizeof(constint_t);
    memcpy(&triple[2], read_from, sizeof(constint_t));
    this->newindex.insert(triple);
  }
};

void get_redist_data(const uint8_t *bytes, size_t len, TripleIndex &repls) {
  const uint8_t *end = bytes + len;
  vector<Rule> rules;
  while (bytes != end) {
    Rule rule;
    bytes = build_condition(bytes, end, rule.condition);
    if (rule.condition.type != CONJUNCTION) {
      cerr << "[ERROR] Expected replication pattern to be a conjunction, but found " << rule.condition << endl;
      return;
    }
    if (rule.condition.get.subformulas.begin->type != ATOMIC) {
      cerr << "[ERROR] First subformula of a replication must be atomic, but found " << *rule.condition.get.subformulas.begin << endl;
    }
    if (rule.condition.get.subformulas.begin->get.atom.type != FRAME &&
        rule.condition.get.subformulas.begin->get.atom.type != ATOM) {
      // Not storing any other data, nor would such occur in RDF,
      // nor can it be inferred (we're not support membership assertion).
      continue;
    }
    Action action;
    action.type = ASSERT_FACT;
    action.get.atom = rule.condition.get.subformulas.begin->get.atom;
    rule.action_block.action_variables.begin = NULL;
    rule.action_block.action_variables.end = NULL;
    rule.action_block.actions.begin = &action;
    rule.action_block.actions.end = rule.action_block.actions.begin + 1;
    TripleIndex ignore;
    infer(rule, repls, ignore);
    // TODO deliberately dropping pointers here, knowing that it should
    // be only a little bit, but this is BAD BAD BAD
    //destroy_condition(rule.condition);
  }
}

// Global variables... shame on you.
bool already_read_replication_conditions = false;
uint8_t *replication_conditions = NULL;
size_t replication_conditions_len = 0;

void redistribute_data(const char *filename) {
  if (MPI::COMM_WORLD.Get_size() <= 1) {
    return;
  }
  if (RANDOMIZE) {
    ZEROSAY("Performing randomization..." << endl);
    Distributor *dist;
    NEW(dist, MPIPacketDistributor, MPI::COMM_WORLD, 3*sizeof(constint_t),
        NUMREQUESTS, COORDEVERY, 382);
    Redistributor *redist;
    NEW(redist, Redistributor, dist);
    redist->randomize = true;
    redist->exec();
    idxpos.swap(redist->newindex);
    DELETE(redist);
  }
  TripleIndex repls (Order(1, 2, 0));
  if (filename != NULL) {

    int world_rank = MPI::COMM_WORLD.Get_rank();
    int world_nproc = MPI::COMM_WORLD.Get_size();

    int local_rank = COMM_LOCAL.Get_rank();
    int local_nproc = COMM_LOCAL.Get_size();

    int repls_rank, repls_nproc;
    if (local_rank == 0) {
      repls_rank = COMM_REPLS.Get_rank();
      repls_nproc = COMM_REPLS.Get_size();
    }

    if (!already_read_replication_conditions) {
      ZEROSAY("Reading the encoded conditions for replication from " << filename << endl);
      replication_conditions = read_all(filename, replication_conditions_len);
      already_read_replication_conditions = true;
    }

    if (replication_conditions == NULL || replication_conditions_len == 0) {
      ZEROSAY("No conditions for replication." << endl);
      return;
    }

//    ZEROSAY("Indexing data for looking up replication data." << endl);
//    // build the indexes to support finding data in need of replication
//    idxspo.insert(idxpos.begin(), idxpos.end());
//    idxosp.insert(idxpos.begin(), idxpos.end());

    ZEROSAY("Query out the data for replication." << endl);
    get_redist_data(replication_conditions, replication_conditions_len, repls);

//    ZEROSAY("Destroying the aforementioned indexes." << endl);
//    // now get rid of them because we need memory during redistribution
//    // and they'll be outdated after redistribution anyway
//    idxpos.clear();
//    idxosp.clear();

    ZEROSAY("Preparing data for replication." << endl);
    int size = repls.size() * 3 * sizeof(constint_t);
    uint8_t *bytes = (uint8_t*)myalloc(3*repls.size(), sizeof(constint_t));
    if (bytes == NULL) {
      cerr << "[ERROR] Processor " << world_rank
           << " failed to allocate " << size
           << " bytes for replicating data." << endl;
      MPI::COMM_WORLD.Abort(-1);
    }

    size_t i, j;
    uint8_t *write_to = bytes;
    TripleIndex::const_iterator it = repls.begin();
    for (; it != repls.end(); ++it) {
      for (i = 0; i < it->size(); ++i) {
        memcpy(write_to, &it->at(i), sizeof(constint_t));
        write_to += sizeof(constint_t);
      }
    }

    ZEROSAY("Doing actual replication." << endl);
    size_t recvbytes;
    int *sizes, *displs;
    uint8_t *recvbuf = NULL;
    if (local_rank == 0) {
      sizes = (int*)myalloc(world_nproc, sizeof(int));
      displs = (int*)myalloc(world_nproc, sizeof(int));
      if (sizes == NULL || displs == NULL) {
        cerr << "[ERROR] Processor " << world_rank
             << " failed to allocate " << 2*world_nproc*sizeof(int)
             << " bytes for gathering each processor's data sizes." << endl;
        MPI::COMM_WORLD.Abort(-2);
      }
    }
    COMM_LOCAL.Gather(&size, 1, MPI::INT, sizes + world_rank, 1, MPI::INT, 0);
    if (local_rank == 0) {
      COMM_REPLS.Allgather(sizes + world_rank, local_nproc, MPI::INT, sizes, local_nproc, MPI::INT);
      displs[0] = 0;
      for (i = 1; i < world_nproc; ++i) {
        displs[i] = displs[i-1] + sizes[i-1];
      }
      recvbytes = displs[world_nproc-1] + sizes[world_nproc-1];
      recvbuf = (uint8_t*)myalloc(recvbytes/sizeof(constint_t),
                                  sizeof(constint_t));
      if (recvbuf == NULL) {
        cerr << "[ERROR] Processor " << world_rank
             << " failed to allocate " << recvbytes
             << " bytes for gathering all the replicated data." << endl;
      }
    }
    COMM_LOCAL.Gatherv(bytes, size, MPI::BYTE, recvbuf,
                       sizes + world_rank, displs + world_rank, MPI::BYTE, 0);
    free(bytes);
    if (local_rank == 0) {
      for (i = 0; i < repls_nproc; ++i) {
        for (j = i; j < local_nproc; ++j) {
          sizes[i] += sizes[j];
        }
        displs[i] = displs[i*local_nproc];
      }
      COMM_REPLS.Allgatherv(recvbuf + displs[repls_rank], sizes[repls_rank], MPI::BYTE,
                            recvbuf, sizes, displs, MPI::BYTE);
      free(sizes);
      free(displs);
    }

    ZEROSAY("Loading replicated data." << endl);
    TripleIndex repl_triples (Order(1, 2, 0));
    if (local_rank == 0) {
      for (i = 0; i < recvbytes; i += 3*sizeof(constint_t)) {
        Triple triple(3);
        for (j = 0; j < 3; ++j) {
          memcpy(&triple[j], recvbuf + i + j*sizeof(constint_t), sizeof(constint_t));
        }
        repl_triples.insert(triple);
      }
      free(recvbuf);
    }

    ZEROSAY("Performing local randomization of replicated data..." << endl);
    Distributor *dist;
    NEW(dist, MPIPacketDistributor, COMM_LOCAL, 3*sizeof(constint_t),
        NUMREQUESTS, COORDEVERY, 382);
    Redistributor *redist;
    NEW(redist, Redistributor, dist, local_rank, local_nproc, repl_triples);
    redist->randomize = true;
    redist->exec();
    repl_triples.clear();
    idxpos.insert(redist->newindex.begin(),
                  redist->newindex.end());
    DELETE(redist);
  }
}

void uniq() {
  if (MPI::COMM_WORLD.Get_size() <= 1) {
    return;
  }
  Distributor *dist;
  NEW(dist, MPIPacketDistributor, MPI::COMM_WORLD, 3*sizeof(constint_t),
      NUMREQUESTS, COORDEVERY, 382);
  DistUniq *redist;
  NEW(redist, DistUniq, dist);
  redist->exec();
  idxpos.swap(redist->newindex);
  DELETE(redist);
  // NOTE intentionally not modifying other indexes because I know this is
  // a last step before output data from idxpos.
}

void write_data(const char *filename) {
  int rank = MPI::COMM_WORLD.Get_rank();
  int commsize = MPI::COMM_WORLD.Get_size();
  string fnamestr(filename);
  size_t hash = fnamestr.find('#');
  if (hash == string::npos && commsize > 1) {
    if (rank == 0) {
      cerr << "[ERROR] File name must contain a '#' to be replaced with processor rank." << endl;
    }
    return;
  }
  if (hash != string::npos) {
    stringstream ss (stringstream::in | stringstream::out);
    ss << fnamestr.substr(0, hash) << rank << fnamestr.substr(hash + 1, fnamestr.size() - hash - 1);
    fnamestr = ss.str();
  }
  size_t unitsize = 3 * sizeof(constint_t);
  size_t bufsize = PAGESIZE - (PAGESIZE % unitsize);
  uint8_t *buffer = (uint8_t*)myalloc(1, bufsize);
  uint8_t *data = (uint8_t*)myalloc(1, bufsize);
  uint8_t *write_to = data;
  uint8_t *end = data + bufsize;
  bool written_once = false;
  MPI::Request req;
  MPI::File file = MPI::File::Open(MPI::COMM_SELF, fnamestr.c_str(),
      MPI::MODE_WRONLY | MPI::MODE_CREATE | MPI::MODE_EXCL, MPI::INFO_NULL);
  file.Seek(0, MPI_SEEK_SET);
  TripleIndex::iterator it = idxpos.begin();
  TripleIndex::iterator endit = idxpos.end();
  for (; it != endit; ++it) {
    Triple::const_iterator tit = it->begin();
    for (; tit != it->end(); ++tit) {
      size_t i;
      for (i = 0; i < sizeof(constint_t); ++i) {
        *write_to = (uint8_t)((*tit >> ((sizeof(constint_t) - i - 1) << 3)) & 0xFF);
        ++write_to;
      }
      if (write_to == end) {
        if (!written_once) {
          written_once = true;
        } else {
          while (!req.Test()) {
            // spin until writing is done
          }
        }
        swap(buffer, data);
        req = file.Iwrite(buffer, bufsize, MPI::BYTE);
        write_to = data;
        end = data + bufsize;
      }
    }
  }
  if (write_to != data) {
    if (written_once) {
      while (!req.Test()) {
        // spin until writing is done
      }
    } else {
      written_once = true;
    }
    req = file.Iwrite(data, write_to - data, MPI::BYTE);
  }
  if (written_once) {
    while (!req.Test()) {
      // spin until writing is done
    }
  }
  file.Close();
  free(buffer);
  free(data);
}

#define FOR_HUMAN_EYES 0
void print_data() {
  TripleIndex::iterator it = idxpos.begin();
  for (; it != idxpos.end(); ++it) {
    Triple::const_iterator tit = it->begin();
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
}





#if defined(TIMING_USE) && TIMING_USE != TIMING_NONE
#if TIMING_USE == TIMING_RDTSC
#define TIMEDIFF_T unsigned long long
#define MPITIME_T MPI::UNSIGNED_LONG_LONG
#else
#define TIMEDIFF_T unsigned long
#define MPITIME_T MPI::UNSIGNED_LONG
#endif
void report_times(const char *time_label, TIME_T(begin), TIME_T(end)) {
  TIMEDIFF_T min, max, sum, sumsq, local, avg, stdev;
  local = DIFFTIME(end, begin);
  MPI::COMM_WORLD.Reduce(&local, &min, 1, MPITIME_T, MPI::MIN, 0);
  MPI::COMM_WORLD.Reduce(&local, &max, 1, MPITIME_T, MPI::MAX, 0);
  MPI::COMM_WORLD.Reduce(&local, &sum, 1, MPITIME_T, MPI::SUM, 0);
  local *= local; // squared
  MPI::COMM_WORLD.Reduce(&local, &sumsq, 1, MPITIME_T, MPI::SUM, 0);
  if (MPI::COMM_WORLD.Get_rank() == 0) {
    double avgd = ((double) sum) / ((double) MPI::COMM_WORLD.Get_size());
    double stdevd = sqrt(((double) sumsq) / ((double) MPI::COMM_WORLD.Get_size()) - (avgd*avgd));
    avg = (TIMEDIFF_T) avgd;
    if (avgd - ((double) avg) > 0.5) {
      ++avg;
    }
    stdev = (TIMEDIFF_T) stdevd;
    if (stdevd - ((double) stdev) > 0.5) {
      ++stdev;
    }
    cerr << "[TIME] " << time_label
         << " Prs " << MPI::COMM_WORLD.Get_size()
         << " Min " << TIMEOUTPUT(min)
         << " Avg " << TIMEOUTPUT(avg)
         << " Max " << TIMEOUTPUT(max)
         << " Std " << TIMEOUTPUT(stdev)
         << endl;
  }
}
#endif


int main(int argc, char **argv) {
  MPI::Init(argc, argv);

  int rank = MPI::COMM_WORLD.Get_rank();

  TIME_T(ts_very_beginning);
  TIMESET(ts_very_beginning);

  if (argc < 4) {
    ZEROSAY("[USAGE] " << argv[0] << " <encoded-rule-file> <encoded-data-file> <output-file> [<encoded-redist-conds-file>]" << endl);
    MPI::Finalize();
    return 0;
  }

  int i, j;

  bool uniquify = false;

#if NPROCS_PER_NODE > 1
  if (MPI::COMM_WORLD.Get_size() % NPROCS_PER_NODE != 0) {
    cerr << "[ERROR] Number of processors " << MPI::COMM_WORLD.Get_size() << " must be a multiple of NPROC_PER_NODE " << NPROCS_PER_NODE << endl;
    MPI::COMM_WORLD.Abort(1);
  }
#endif

  int *local_ranks = (int*)myalloc(NPROCS_PER_NODE, sizeof(int));
  int base_rank = rank / NPROCS_PER_NODE;
  base_rank *= NPROCS_PER_NODE;
  for (i = 0; i < NPROCS_PER_NODE; ++i) {
    local_ranks[i] = base_rank + i;
  }
  MPI::Group group_world = MPI::COMM_WORLD.Get_group();
  MPI::Group group_local = group_world.Incl(NPROCS_PER_NODE, local_ranks);
  free(local_ranks);
  COMM_LOCAL = MPI::COMM_WORLD.Create(group_local);
  int repl_nprocs = MPI::COMM_WORLD.Get_size() / NPROCS_PER_NODE;
  int *repl_ranks = (int*)myalloc(repl_nprocs, sizeof(int));
  int offset = rank % NPROCS_PER_NODE;
  for (i = 0; i < repl_nprocs; ++i) {
    repl_ranks[i] = i * NPROCS_PER_NODE + offset;
  }
  MPI::Group group_repls = group_world.Incl(repl_nprocs, repl_ranks);
  free(repl_ranks);
  COMM_REPLS = MPI::COMM_WORLD.Create(group_repls);

  srandom(rank);
  j = 0;
  for (i = 4; i < argc; ++i) {
    if (strcmp(argv[i], "--packet-size") == 0) {
      stringstream ss(stringstream::in | stringstream::out);
      ss << argv[++i];
      ss >> PACKETSIZE; // currently unused
    } else if (strcmp(argv[i], "--num-requests") == 0) {
      stringstream ss(stringstream::in | stringstream::out);
      ss << argv[++i];
      ss >> NUMREQUESTS;
    } else if (strcmp(argv[i], "--check-every") == 0) {
      stringstream ss(stringstream::in | stringstream::out);
      ss << argv[++i];
      ss >> COORDEVERY;
    } else if (strcmp(argv[i], "--uniq") == 0 || strcmp(argv[i], "-u") == 0) {
      uniquify = true;
    } else if (strcmp(argv[i], "--page-size") == 0) {
      stringstream ss(stringstream::in | stringstream::out);
      ss << argv[++i];
      ss >> PAGESIZE;
    } else if (strcmp(argv[i], "--randomize") == 0) {
      RANDOMIZE = true;
    } else if (strcmp(argv[i], "--complete") == 0) {
      COMPLETE = true;
    }
  }

  ZEROSAY("[INFO] PACKETSIZE: " << PACKETSIZE << endl);
  ZEROSAY("[INFO] NUMREQUESTS: " << NUMREQUESTS << endl);
  ZEROSAY("[INFO] COORDEVERY: " << COORDEVERY << endl);
  ZEROSAY("[INFO] UNIQUIFY: " << uniquify << endl);
  ZEROSAY("[INFO] PAGESIZE: " << PAGESIZE << endl);
  ZEROSAY("[INFO] RANDOMIZE: " << RANDOMIZE << endl);

  TIME_T(ts_load_rules);
  TIMESET(ts_load_rules);

  vector<Rule> rules;

  ZEROSAY("[INFO] Loading rules from " << argv[1] << endl);
  load_rules(argv[1], rules);

  //print_rules(rules);

  TIME_T(ts_load_data);
  TIMESET(ts_load_data);

  ZEROSAY("[INFO] Loading data from " << argv[2] << endl);
  load_data(argv[2]);

  TIME_T(ts_randomize);
  TIMESET(ts_randomize);

  if (RANDOMIZE || argc > 4) {
    ZEROSAY("[INFO] Redistributing data according to " << (argc <= 4 ? "(nothing)" : argv[4]) << endl);
    redistribute_data(argc > 4 ? argv[4] : NULL);
  }

  TIME_T(ts_index);
  TIMESET(ts_index);

  ZEROSAY("[INFO] Building indexes." << endl);
#if USE_INDEX_SPO
  idxspo.insert(idxpos.begin(), idxpos.end());
#endif
#if USE_INDEX_OSP
  idxosp.insert(idxpos.begin(), idxpos.end());
#endif

  TIME_T(ts_infer);
  TIMESET(ts_infer);

  vector<size_t> old_sizes, new_sizes;
  note_sizes(new_sizes);

  uint8_t another_iteration = 0;
  do {
    ZEROSAY("[INFO] Inferring..." << endl);
    infer(rules);

    another_iteration = false;
    if (COMPLETE && argc > 4) {
      vector<size_t> before, after;
      note_sizes(before);
      
      ZEROSAY("[INFO] Redistributing data for completeness." << endl);
      RANDOMIZE = false;
      redistribute_data(argv[4]);

      note_sizes(after);
      uint8_t local_need_another_iteration = (before != after) ? 1 : 0;
      MPI::COMM_WORLD.Allreduce(&local_need_another_iteration,
          &another_iteration, 1,  MPI::BYTE, MPI::BOR);
    }
  } while (another_iteration != 0);

  TIME_T(ts_destroy_index);
  TIMESET(ts_destroy_index);

  ZEROSAY("[INFO] Destroying indexes." << endl);
  idxspo.clear();
  idxosp.clear();
  map<constint_t, Index>::iterator it = atoms.begin();
  for (; it != atoms.end(); ++it) {
    // Save rif:error tuple (there can be only one) to check for
    // inconsistency later.
    if (it->first != CONST_RIF_ERROR) {
      it->second.clear();
    }
  }

  TIME_T(ts_uniq);
  TIMESET(ts_uniq);

  if (uniquify) {
    ZEROSAY("[INFO] Deduplicating data." << endl);
    uniq();
  }

  TIME_T(ts_output);
  TIMESET(ts_output);

  ZEROSAY("[INFO] Writing output to " << argv[3] << endl);
  write_data(argv[3]);

  TIME_T(ts_finish);
  TIMESET(ts_finish);

#if defined(TIMING_USE) && TIMING_USE != TIMING_NONE
  report_times("Loading rules", ts_load_rules, ts_load_data);
  report_times("Loading data", ts_load_data, ts_randomize);
  report_times("Randomize", ts_randomize, ts_index);
  report_times("Index", ts_index, ts_infer);
  report_times("Infer", ts_infer, ts_destroy_index);
  report_times("Unindex", ts_destroy_index, ts_uniq);
  report_times("Unique", ts_uniq, ts_output);
  report_times("Output", ts_output, ts_finish);
  report_times("Overall", ts_very_beginning, ts_finish);
#endif

  MPI::Finalize();
  return 0;
}
