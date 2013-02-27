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
