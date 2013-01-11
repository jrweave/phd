#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include "ex/TraceableException.h"
#include "iri/IRIRef.h"
#include "main/encode.h"
#include "ptr/DPtr.h"
#include "ptr/MPtr.h"
#include "rdf/RDFTerm.h"
#include "rif/RIFAction.h"
#include "rif/RIFActionBlock.h"
#include "rif/RIFAtomic.h"
#include "rif/RIFCondition.h"
#include "rif/RIFConst.h"
#include "rif/RIFRule.h"
#include "rif/RIFTerm.h"
#include "rif/RIFVar.h"
#include "sys/endian.h"
#include "util/funcs.h"

#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG 0

#define HACK_CONST_IDS 1

using namespace ex;
using namespace iri;
using namespace ptr;
using namespace rdf;
using namespace rif;
using namespace std;
using namespace sys;
using namespace util;

typedef map<RIFVar, varint_t, bool(*)(const RIFVar &, const RIFVar &)> VarEncMap;
typedef map<RIFConst, constint_t, bool(*)(const RIFConst &, const RIFConst &)> ConstEncMap;
typedef map<RIFConst, builtint_t, bool(*)(const RIFConst &, const RIFConst &)> BuiltEncMap;
typedef map<RIFConst, funcint_t, bool(*)(const RIFConst &, const RIFConst &)> FuncEncMap;
typedef map<RIFConst, execint_t, bool(*)(const RIFConst &, const RIFConst &)> ExecEncMap;

// GLOBAL VARIABLES ARE EVIL!... but this is already such a hack... oh well
ConstEncMap constenc (RIFConst::cmplt0);
BuiltEncMap builtenc (RIFConst::cmplt0);
FuncEncMap funcenc (RIFConst::cmplt0);
ExecEncMap execenc (RIFConst::cmplt0);

void print_encoded(list<uint8_t> &encoded);

DPtr<uint8_t> *cstr2dptr(const char *cstr) {
  size_t len = strlen(cstr);
  DPtr<uint8_t> *p;
  NEW(p, MPtr<uint8_t>, len);
  ascii_strncpy(p->dptr(), cstr, len);
  return p;
}

template<typename K, typename V, typename CMP>
void support_map(map<K, V, CMP> &m, K k, V v) {
  typename map<K, V, CMP>::const_iterator it = m.find(k);
  if (it == m.end()) {
    m.insert(pair<K,V>(k, v));
  } else if (it->second != v) {
    THROW(TraceableException, "Double mapping.");
  }
}

template<typename int_t, typename CMP>
void support_map_constant(map<RIFConst, int_t, CMP> &m, const char *cstr, int_t n) {
  DPtr<uint8_t> *p = cstr2dptr(cstr);
  RIFConst c = RIFConst::parse(p);
  p->drop();
  support_map(m, c, n);
}

template<typename int_t>
void append_big_endian_int(list<uint8_t> &l, const int_t n) {
  size_t i;
  for (i = 0; i < sizeof(int_t); ++i) {
    l.push_back((uint8_t)((n >> ((sizeof(int_t) - i - 1) << 3)) & 0xFF));
  }
}

void prepend_big_endian_size(list<uint8_t> &l) THROWS(TraceableException) {
  if (l.size() > SIZEINT_MAX) {
    THROW(TraceableException, "Encoding is too long for given sizeint_t.");
  }
  sizeint_t size = (sizeint_t) l.size();
  size_t i;
  for (i = 0; i < sizeof(sizeint_t); ++i) {
    l.push_front((uint8_t)((size >> (i << 3)) & 0xFF));
  }
}
TRACE(TraceableException, "(trace)")

// ===== ENCODED VARIABLE =====
// |-V--|
void encode_variable(list<uint8_t> &encoded, const RIFVar &var, VarEncMap &varenc) THROWS(TraceableException) {
  VarEncMap::const_iterator it = varenc.find(var);
  if (it != varenc.end()) {
    append_big_endian_int(encoded, it->second);
    return;
  }
  if (varenc.size() > VARINT_MAX || varenc.size() >= varenc.max_size()) {
    THROW(TraceableException, "Too many variables to be encoded.");
  }
  varint_t vint = (varint_t) varenc.size();
  varenc[var] = vint;
  append_big_endian_int(encoded, vint);
}
TRACE(TraceableException, "(trace)")

// ===== ENCODED CONSTANT =====
// |-C--|
void encode_constant(list<uint8_t> &encoded, const RIFConst &c) THROWS(TraceableException) {
  if (constenc.empty()) {
    // HARD CODE CONSTANTS HERE
    DPtr<uint8_t> *p = cstr2dptr("\"http://www.w3.org/1999/02/22-rdf-syntax-ns#first\"^^<http://www.w3.org/2007/rif#iri>");
    constenc[RIFConst::parse(p)] = CONST_RDF_FIRST;
    p->drop();
    p = cstr2dptr("\"http://www.w3.org/1999/02/22-rdf-syntax-ns#rest\"^^<http://www.w3.org/2007/rif#iri>");
    constenc[RIFConst::parse(p)] = CONST_RDF_REST;
    p->drop();
    p = cstr2dptr("\"http://www.w3.org/1999/02/22-rdf-syntax-ns#nil\"^^<http://www.w3.org/2007/rif#iri>");
    constenc[RIFConst::parse(p)] = CONST_RDF_NIL;
    p->drop();
    p = cstr2dptr("\"http://www.w3.org/2007/rif#error\"^^<http://www.w3.org/2007/rif#iri>");
    constenc[RIFConst::parse(p)] = CONST_RIF_ERROR;
    p->drop();
  }
  ConstEncMap::const_iterator it = constenc.find(c);
  if (it != constenc.end()) {
    append_big_endian_int(encoded, it->second);
    return;
  }
  if (constenc.size() > CONSTINT_MAX || constenc.size() >= constenc.max_size()) {
    THROW(TraceableException, "Too many constants to be encoded.");
  }
  constint_t cint = (constint_t) constenc.size();

#if HACK_CONST_IDS
  ///// HACK! /////
  ++cint;
  cint |= ((constint_t)1) << ((sizeof(constint_t) << 3) - 1);
  ///// HACK! /////
#endif

  constenc[c] = cint;
  append_big_endian_int(encoded, cint);
}
TRACE(TraceableException, "(trace)")

// ===== ENCODED BUILTIN PREDICATE =====
// |-B--|
void encode_builtin_predicate(list<uint8_t> &encoded, const RIFConst &name) THROWS(TraceableException) {
  if (builtenc.empty()) {
    // INITIALIZE SUPPORTED BUILTINS HERE
    support_map_constant(builtenc, "\"http://www.w3.org/2007/rif-builtin-predicate#list-contains\"^^<http://www.w3.org/2007/rif#iri>", BUILTIN_PRED_LIST_CONTAINS);
  }
  BuiltEncMap::const_iterator it = builtenc.find(name);
  if (it == builtenc.end()) {
    THROW(TraceableException, "Unsupported builtin.");
  }
  append_big_endian_int(encoded, it->second);
}
TRACE(TraceableException, "(trace)")

// ===== ENCODED FUNCTION NAME =====
// |-F--|
void encode_function_name(list<uint8_t> &encoded, const RIFConst &name) THROWS(TraceableException) {
  if (funcenc.empty()) {
    // INITIALIZE SUPPORTED FUNCTIONS HERE
  }
  FuncEncMap::const_iterator it = funcenc.find(name);
  if (it == funcenc.end()) {
    THROW(TraceableException, "Unsupported function.");
  }
  append_big_endian_int(encoded, it->second);
}
TRACE(TraceableException, "(trace)")

void encode_term(list<uint8_t> &encoded, const RIFTerm &term, VarEncMap &varenc) throw(TraceableException);

// ===== ENCODED LIST =====
//  +------------------------ #bytes(term_1) + ... + #bytes(term_N)
//  |    +------------------- term_1
//  |    |                    ...
//  |    |                 +- term_N
//  v    v                 v
// |-S--|--- ... ---| ... |--- ... ---|
void encode_list(list<uint8_t> &encoded, const RIFTerm &l, VarEncMap &varenc) THROWS(TraceableException) {
  if (l.getType() != LIST) {
    THROW(TraceableException, "This should never happen.");
  }
  list<uint8_t> encoded_list;
  DPtr<RIFTerm> *items = l.getItems();
  RIFTerm *item = items->dptr();
  RIFTerm *end = item + items->size();
  for (; item != end; ++item) {
    encode_term(encoded_list, *item, varenc);
  }
  items->drop();
  prepend_big_endian_size(encoded_list);
#if DEBUG
  cout << "LIST: ";
  print_encoded(encoded_list);
#endif
  encoded.splice(encoded.end(), encoded_list);
}
TRACE(TraceableException, "(trace)")

// ===== ENCODED FUNCTION =====
//  +----------------------------- F + #bytes(argterm_1) + ...
//  |                              + #bytes(argterm_N)
//  |    +------------------------ funcname
//  |    |    +------------------- argterm_1
//  |    |    |                    ...
//  |    |    |                 +- argterm_N
//  v    v    v                 v
// |-S--|-F--|--- ... ---| ... |--- ... ---|
void encode_function(list<uint8_t> &encoded, const RIFTerm &func, VarEncMap &varenc) THROWS(TraceableException) {
  if (func.getType() != FUNCTION) {
    THROW(TraceableException, "This should never happen.");
  }
  list<uint8_t> encoded_function;
  encode_function_name(encoded_function, func.getPred());
  DPtr<RIFTerm> *args = func.getArgs();
  RIFTerm *arg = args->dptr();
  RIFTerm *end = arg + args->size();
  for (; arg != end; ++arg) {
    encode_term(encoded_function, *arg, varenc);
  }
  args->drop();
  prepend_big_endian_size(encoded_function);
#if DEBUG
  cout << "FUNCTION: ";
  print_encoded(encoded_function);
#endif
  encoded.splice(encoded.end(), encoded_function);
}
TRACE(TraceableException, "(trace)")

// ===== ENCODED TERM TYPE =====
// |-TT-|
void encode_term_type(list<uint8_t> &encoded, const enum RIFTermType type) {
  termtypeint_t ttint = (termtypeint_t) type;
  append_big_endian_int(encoded, ttint);
}

// ===== ENCODED TERM =====
//  +------ type
//  |    +- payload
//  v    v
// |-TT-|--- ... ---|
void encode_term(list<uint8_t> &encoded, const RIFTerm &term, VarEncMap &varenc) THROWS(TraceableException) {
  encode_term_type(encoded, term.getType());
  switch (term.getType()) {
    case VARIABLE:
      encode_variable(encoded, term.getVar(), varenc);
      break;
    case CONSTANT:
      encode_constant(encoded, term.getConst());
      break;
    case LIST:
      encode_list(encoded, term, varenc);
      break;
    case FUNCTION:
      encode_function(encoded, term, varenc);
      break;
    default: cerr << "[ERROR] " << term.getType() << endl; THROW(TraceableException, "Unhandled case.");
  }
}
TRACE(TraceableException, "(trace)")

// ===== ENCODED FRAME =====
//  +------------------------------------------------------- objterm
//  |           +------------------------------------------- attrterm_1
//  |           |           +------------------------------- valterm_1
//  |           |           |                                ...
//  |           |           |                 +------------- attrterm_N
//  |           |           |                 |           +- valterm_N
//  v           v           v                 v           v
// |--- ... ---|--- ... ---|--- ... ---| ... |--- ... ---|--- ... ---|
//
// NOTE: Frame is treated a bit special here in the code.
// It does not prepend its encodings with a size.  If encode_frame
// is called from encode_atom, then encode_atom will prepend an
// appropriate size for the length of the entire atomic formula.
// If encode_frame is called from encode_action_variable_binding,
// then an appropriate size for the length of the entire action
// variable binding is used.  encode_frame should not be called
// from anywhere else.
void encode_frame(list<uint8_t> &encoded, const RIFAtomic &frame, VarEncMap &varenc) THROWS(TraceableException) {
  if (frame.getType() != FRAME) {
    THROW(TraceableException, "This should never happen.");
  }
  encode_term(encoded, frame.getObject(), varenc);
  TermSet attrs (RIFTerm::cmplt0);
  frame.getAttrs(attrs);
  TermSet::const_iterator it = attrs.begin();
  for (; it != attrs.end(); ++it) {
    TermSet vals (RIFTerm::cmplt0);
    frame.getValues(*it, vals);
    TermSet::const_iterator it2 = vals.begin();
    for (; it2 != vals.end(); ++it2) {
      encode_term(encoded, *it, varenc);
      encode_term(encoded, *it2, varenc);
    }
  }
}
TRACE(TraceableException, "(trace)")

// ===== ENCODED ATOMIC TYPE =====
// |-AT-|
void encode_atomic_type(list<uint8_t> &encoded, const enum RIFAtomicType type) {
  atomtypeint_t atint = (atomtypeint_t) type;
  append_big_endian_int(encoded, atint);
}

// ===== ENCODED ATOMIC =====
//  +----------- #bytes(type) + #bytes(payload)
//  |    +------ type
//  |    |    +- payload
//  v    v    v
// |-S--|-AT-|------------------ ... ------------------|
//
//  +----------------------------------------- #bytes(type) + #bytes(pred)
//  |                                          + #bytes(term_1) + ...
//  |                                          + #bytes(term_N)
//  |    +------------------------------------ type = ATOM | EXTERNAL
//  |    |    +------------------------------- pred
//  |    |    |           +------------------- term_1
//  |    |    |           |                    ...
//  |    |    |           |                 +- term_N
//  v    v    v           v                 v
// |-S--|-AT-|--- ... ---|--- ... ---| ... |--- ... ---|
//
//  +-------------------------------- #bytes(type) + #bytes(lhsterm)
//  |                                 + #bytes(rhsterm)
//  |    +--------------------------- type = EQUALITY | MEMBERSHIP | SUBCLASS
//  |    |    +---------------------- lhsterm
//  |    |    |                    +- rhsterm
//  v    v    v                    v
// |-S--|-AT-|-------- ... -------|-------- ... -------|
//
// NOTE: type = FRAME is handled specially.  See encode_frame.
void encode_atomic(list<uint8_t> &encoded, const RIFAtomic &atom, VarEncMap &varenc) THROWS(TraceableException) {
  list<uint8_t> encoded_atom;
  encode_atomic_type(encoded_atom, atom.getType());
  switch (atom.getType()) {
    case ATOM: {
      encode_constant(encoded_atom, atom.getPred());
      DPtr<RIFTerm> *args = atom.getArgs();
      RIFTerm *arg = args->dptr();
      RIFTerm *end = arg + args->size();
      for (; arg != end; ++arg) {
        encode_term(encoded_atom, *arg, varenc);
      }
      args->drop();
      break;
    }
    case EXTERNAL: {
      encode_builtin_predicate(encoded_atom, atom.getPred());
      DPtr<RIFTerm> *args = atom.getArgs();
      RIFTerm *arg = args->dptr();
      RIFTerm *end = arg + args->size();
      for (; arg != end; ++arg) {
        encode_term(encoded_atom, *arg, varenc);
      }
      args->drop();
      break;
    }
    case EQUALITY: {
#if DEBUG
      cout << "[DEBUG] Encoding equality formula." << endl;
#endif
      pair<RIFTerm, RIFTerm> eq = atom.getEquality();
      encode_term(encoded_atom, eq.first, varenc);
      encode_term(encoded_atom, eq.second, varenc);
      break;
    }
    case MEMBERSHIP: {
      encode_term(encoded_atom, atom.getObject(), varenc);
      encode_term(encoded_atom, atom.getClass(), varenc);
      break;
    }
    case SUBCLASS: {
      encode_term(encoded_atom, atom.getSubclass(), varenc);
      encode_term(encoded_atom, atom.getSuperclass(), varenc);
      break;
    }
    case FRAME: {
      encode_frame(encoded_atom, atom, varenc);
      break;
    }
    default: THROW(TraceableException, "Unhandled case.");
  }
  prepend_big_endian_size(encoded_atom);
#if DEBUG
  cout << "ATOMIC: ";
  print_encoded(encoded_atom);
#endif
  encoded.splice(encoded.end(), encoded_atom);
}
TRACE(TraceableException, "(trace)")

// ===== ENCODED EXECUTION PREDICATE =====
// |-E--|
void encode_execute_predicate(list<uint8_t> &encoded, const RIFConst &name) THROWS(TraceableException) {
  if (execenc.empty()) {
    // INITIALIZE SUPPORTED EXECUTION HERE
  }
  ExecEncMap::const_iterator it = execenc.find(name);
  if (it == execenc.end()) {
    THROW(TraceableException, "Unsupported execution.");
  }
  append_big_endian_int(encoded, it->second);
}
TRACE(TraceableException, "(trace)")

// ===== ENCODED ACTION TYPE =====
// |ACT-|
void encode_action_type(list<uint8_t> &encoded, const enum RIFActType type) {
  acttypeint_t atint = (acttypeint_t) type;
  append_big_endian_int(encoded, atint);
}

// ===== ENCODED ACTION =====
//  +------ type
//  |    +- payload
//  v    v
// |ACT-|--- ... ---|
//
//  +------ type = ASSERT_FACT | RETRACT_FACT | MODIFY
//  |    +- atomic target
//  v    v
// |ACT-|--- ... ---|
//
//  +------------------ type = RETRACT_SLOTS
//  |    +------------- object term
//  |    |           +- attribute term
//  v    v           v
// |ACT-|--- ... ---|--- ... ---|
//
//  +------ type = RETRACT_OBJECT
//  |    +- object term
//  v    v
// |ACT-|--- ... ---|
//
//  +----------------------------- type = EXECUTE
//  |    +------------------------ pred
//  |    |    +------------------- argterm_1
//  |    |    |                    ...
//  |    |    |                 +- argterm_N
//  v    v    v                 v
// |ACT-|-E--|--- ... ---| ... |--- ... ---|
void encode_action(list<uint8_t> &encoded, const RIFAction &action, VarEncMap &varenc) THROWS(TraceableException) {
  encode_action_type(encoded, action.getType());
  switch (action.getType()) {
    case ASSERT_FACT:
    case RETRACT_FACT:
    case MODIFY:
      encode_atomic(encoded, action.getTargetAtomic(), varenc);
      break;
    case RETRACT_SLOTS: {
      pair<RIFTerm, RIFTerm> p = action.getTargetTermPair();
      encode_term(encoded, p.first, varenc);
      encode_term(encoded, p.second, varenc);
      break;
    }
    case RETRACT_OBJECT: {
      encode_term(encoded, action.getTargetTerm(), varenc);
      break;
    }
    case EXECUTE: {
      // TODO if action is Execute() (no-op, by default constructor),
      // then this will throw an exception.  Need to add method to
      // RIFAction for checking whether it is a no-op.
      RIFAtomic target = action.getTargetAtomic();
      encode_execute_predicate(encoded, target.getPred());
      DPtr<RIFTerm> *args = target.getArgs();
      RIFTerm *arg = args->dptr();
      RIFTerm *end = arg + args->size();
      for (; arg != end; ++arg) {
        encode_term(encoded, *arg, varenc);
      }
      args->drop();
      break;
    }
  }
}
TRACE(TraceableException, "(trace)")

// ===== ENCODED ACTION VARIABLE BINDING =====
//  +-------------------- #bytes(variable) + 1 + #bytes(atomic)
//  |    +--------------- variable
//  |    |           +--- "newness" (0 if New(), 1 otherwise)
//  |    |           | +- atomic (empty if newness = 0)
//  v    v           v v
// |-S--|--- ... ---|1|--- ... ---|
void encode_action_variable_binding(list<uint8_t> &encoded, const RIFActVarBind &actvarbind, VarEncMap &varenc) THROWS(TraceableException) {
  list<uint8_t> encoded_actvarbind;
  encode_variable(encoded_actvarbind, actvarbind.getActVar(), varenc);
  if (actvarbind.isNew()) {
    encoded_actvarbind.push_back(UINT8_C(0));
  } else {
    encoded_actvarbind.push_back(UINT8_C(1));
    encode_frame(encoded_actvarbind, actvarbind.getFrame(), varenc);
  }
  prepend_big_endian_size(encoded_actvarbind);
  encoded.splice(encoded.end(), encoded_actvarbind);
}
TRACE(TraceableException, "(trace)")

// ===== ENCODED ACTION VARIABLE DECLARATION =====
//  +------------------------ #bytes(actvarbind_1) + ... + #bytes(actvarbind_N)
//  |    +------------------- actvarbind_1
//  |    |                    ...
//  |    |                 +- actvarbind_N
//  v    v                 v
// |-S--|--- ... ---| ... |--- ... ---|
void encode_action_variable_declarations(list<uint8_t> &encoded, const RIFActionBlock &action_block, VarEncMap &varenc) THROWS(TraceableException) {
  list<uint8_t> encoded_actvars;
  VarSet actvars(RIFVar::cmplt0);
  action_block.getActVars(&actvars);
  VarSet::const_iterator actvar = actvars.begin();
  for (; actvar != actvars.end(); ++actvar) {
    RIFActVarBind actvarbind = action_block.getBinding(*actvar);
    encode_action_variable_binding(encoded_actvars, actvarbind, varenc);
  }
  prepend_big_endian_size(encoded_actvars);
  encoded.splice(encoded.end(), encoded_actvars);
}
TRACE(TraceableException, "(trace)")

// ===== ENCODED ACTION BLOCK =====
//  +------------------------------------ #bytes(action_1) + ... + #bytes(action_N)
//  |    +------------------------------- action variable declarations
//  |    |           +------------------- action_1
//  |    |           |                    ...
//  |    |           |                 +- action_N
//  v    v           v                 v
// |-S--|--- ... ---|--- ... ---| ... |--- ... ---|
void encode_action_block(list<uint8_t> &encoded, const RIFActionBlock &action_block, VarEncMap &varenc) THROWS(TraceableException) {
  list<uint8_t> encoded_action_block;
  encode_action_variable_declarations(encoded_action_block, action_block, varenc);
  DPtr<RIFAction> *actions = action_block.getActions();
  RIFAction *action = actions->dptr();
  RIFAction *end = action + actions->size();
  for (; action != end; ++action) {
    encode_action(encoded_action_block, *action, varenc);
  }
  actions->drop();
  prepend_big_endian_size(encoded_action_block);
#if DEBUG
  cout << "ACTION BLOCK: ";
  print_encoded(encoded_action_block);
#endif
  encoded.splice(encoded.end(), encoded_action_block);
}
TRACE(TraceableException, "(trace)")

// ===== ENCODED CONDITION TYPE =====
// |CND-|
void encode_condition_type(list<uint8_t> &encoded, const enum RIFCondType type, VarEncMap &varenc) {
  condtypeint_t ctint = (condtypeint_t) type;
  append_big_endian_int(encoded, ctint);
}

// ===== ENCODED CONDITION =====
//  +----------- CND + #bytes(payload)
//  |    +------ type
//  |    |    +- payload
//  v    v    v
// |-S--|CND-|--- ... ---|
//
//  +----------- CND + #bytes(atomic)
//  |    +------ type = ATOMIC
//  |    |    +- atomic
//  v    v    v
// |-S--|CND-|--- ... ---|
//
//  +----------------------------- CND + #bytes(sub_1) + ... + #bytes(sub_N)
//  |    +------------------------ type = CONJUNCTION | DISJUNCTION
//  |    |    +------------------- sub_1
//  |    |    |                    ...
//  |    |    |                 +- sub_N
//  v    v    v                 v
// |-S--|CND-|--- ... ---| ... |--- ... ---|
//
//  +----------- CND + #bytes(sub)
//  |    +------ type = NEGATION
//  |    |    +- sub
//  v    v    v
// |-S--|CND-|--- ... ---|
//
// TODO got lazy for type = EXISTENTIAL
void encode_condition(list<uint8_t> &encoded, const RIFCondition &cond, VarEncMap &varenc) THROWS(TraceableException) {
  list<uint8_t> encoded_cond;
  encode_condition_type(encoded_cond, cond.getType(), varenc);
  switch (cond.getType()) {
    case ATOMIC:
      encode_atomic(encoded_cond, cond.getAtomic(), varenc);
      break;
    case CONJUNCTION:
    case DISJUNCTION: {
      DPtr<RIFCondition> *subs = cond.getSubformulas();
      RIFCondition *sub = subs->dptr();
      RIFCondition *end = sub + subs->size();
      for (; sub != end; ++sub) {
        encode_condition(encoded_cond, *sub, varenc);
      }
      subs->drop();
      break;
    }
    case NEGATION: {
      encode_condition(encoded_cond, cond.getSubformula(), varenc);
      break;
    }
    case EXISTENTIAL: {
      THROW(TraceableException, "I just got lazy.  Tired of this."); // TODO
    }
    default: {
      THROW(TraceableException, "Unhandled case.");
    }
  }
  prepend_big_endian_size(encoded_cond);
#if DEBUG
  cout << "CONDITION: ";
  print_encoded(encoded_cond);
#endif
  encoded.splice(encoded.end(), encoded_cond);
}
TRACE(TraceableException, "(trace)")

//  ===== ENCODED RULE =====
//  +------------------ #bytes(condition) + #bytes(action block)
//  |    +------------- condition
//  |    |           +- action block
//  v    v           v
// |-S--|--- ... ---|--- ... ---|
void encode_rule(list<uint8_t> &encoded, const RIFRule &rule) THROWS(TraceableException) {
  VarEncMap varenc (RIFVar::cmplt0);
  encode_condition(encoded, rule.getCondition(), varenc);
  encode_action_block(encoded, rule.getActionBlock(), varenc);
  prepend_big_endian_size(encoded);
}
TRACE(TraceableException, "(trace)")

void print_encoded(list<uint8_t> &encoded, ostream &out) {
#if DEBUG
  out << setfill('0');
  list<uint8_t>::const_iterator it = encoded.begin();
  for (; it != encoded.end(); ++it) {
   out << setw(2) << hex << (int)*it << ' ';
  }
  out << endl;
#else
  list<uint8_t>::const_iterator it = encoded.begin();
  for (; it != encoded.end(); ++it) {
    out.write((const char*)(&*it), 1);
  }
#endif
}

void print_encoded(list<uint8_t> &encoded) {
  print_encoded(encoded, cout);
}

RIFCondition parse_condition(string str) THROWS(TraceableException) {
  DPtr<uint8_t> *p = cstr2dptr(str.c_str());
  RIFCondition condition = RIFCondition::parse(p);
  p->drop();
  return condition;
}
TRACE(TraceableException, "(trace)")

RIFRule parse_rule(string str) THROWS(TraceableException) {
  DPtr<uint8_t> *p = cstr2dptr(str.c_str());
  RIFRule rule = RIFRule::parse(p);
  p->drop();
  return rule;
}
TRACE(TraceableException, "(trace)")

RDFTerm rif_to_rdf(const RIFConst &rifconst) {
  IRIRef dtiri = rifconst.getDatatype();
  DPtr<uint8_t> *dtstr = dtiri.getUTF8String();
  size_t len = strlen("http://www.w3.org/2007/rif#iri");
  if (dtstr->size() == len && ascii_strncmp(dtstr->dptr(), "http://www.w3.org/2007/rif#iri", len) == 0) {
    dtstr->drop();
    DPtr<uint8_t> *lexform = rifconst.getLexForm();
    IRIRef iri (lexform);
    lexform->drop();
    return RDFTerm(iri);
  }
  len = strlen("http://www.w3.org/2001/XMLSchema#string");
  if (dtstr->size() == len && ascii_strncmp(dtstr->dptr(), "http://www.w3.org/2001/XMLSchema#string", len) == 0) {
    dtstr->drop();
    DPtr<uint8_t> *lexform = rifconst.getLexForm();
    RDFTerm rdfterm(lexform, NULL);
    lexform->drop();
    return rdfterm;
  }
  len = strlen("http://www.w3.org/1999/02/22-rdf-syntax-ns#PlainLiteral");
  if (dtstr->size() == len && ascii_strncmp(dtstr->dptr(), "http://www.w3.org/1999/02/22-rdf-syntax-ns#PlainLiteral", len) == 0) {
    DPtr<uint8_t> *lexform = rifconst.getLexForm();
    const uint8_t *begin = lexform->dptr();
    const uint8_t *end = begin + lexform->size();
    const uint8_t *at = end;
    while (at != begin) {
      --at;
      if (*at == to_ascii('@')) {
        break;
      }
    }
    if (at != end && *at == to_ascii('@')) {
      dtstr->drop();
      DPtr<uint8_t> *lexpart = lexform->sub(0, at - begin);
      LangTag *langtag = NULL;
      if (at != end - 1) {
        DPtr<uint8_t> *langpart = lexform->sub((at - begin) + 1, (end - at) - 1);
        NEW(langtag, LangTag, langpart);
        langpart->drop();
      }
      RDFTerm rdfterm(lexpart, langtag);
      lexpart->drop();
      if (langtag != NULL) {
        DELETE(langtag);
      }
      return rdfterm;
    }
  }
  dtstr->drop();
  DPtr<uint8_t> *lexform = rifconst.getLexForm();
  RDFTerm rdfterm(lexform, dtiri);
  lexform->drop();
  return rdfterm;
}

void print_constants() {
#if DEBUG
  cerr << "[DEBUG] Print constants..." << endl;
#endif
  ConstEncMap::const_iterator it = constenc.begin();
  for (; it != constenc.end(); ++it) {
    if (is_little_endian()) {
      constint_t cint = it->second;
      reverse_bytes(cint);
      cerr.write((const char*)(&cint), sizeof(constint_t));
    } else {
      cerr.write((const char*)(&it->second), sizeof(constint_t));
    }
    DPtr<uint8_t> *str = rif_to_rdf(it->first).toUTF8String();
    uint32_t len = (uint32_t)str->size();
    if (is_little_endian()) {
      reverse_bytes(len);
    }
    cerr.write((const char*)(&len), sizeof(uint32_t));
    cerr.write((const char*)str->dptr(), str->size());
    str->drop();
  }
}

int doit(int argc, char **argv) THROWS(TraceableException) {
#if DEBUG
  cerr << "[DEBUG] LINE " << __LINE__ << endl;
#endif
  ofstream fout;
  string filename;
  bool do_print_constants = false;
  int i;
  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--print-constants") == 0) {
      do_print_constants = true;
    } else if (strcmp(argv[i], "--force") == 0) {
      stringstream ss(stringstream::in | stringstream::out);
      ss << argv[++i];
      constint_t c;
      ss >> hex >> c;
      cerr << "[INFO] Mapping " << argv[i+1] << " to " << hex << c << endl;
      DPtr<uint8_t> *p = cstr2dptr(argv[++i]);
      RIFConst cnst = RIFConst::parse(p);
      p->drop();
      if (constenc.count(cnst) > 0) {
        cerr << "[WARNING] Forcing duplicate mapping to same term.  The latest mapping will be used." << endl;
      }
      constenc[cnst] = c;
    } else {
      filename = argv[i];
    }
  }
#if DEBUG
  cerr << "[DEBUG] LINE " << __LINE__ << endl;
#endif
  if (filename.size() > 0) {
    fout.open(filename.c_str());
  }
#if DEBUG
  cerr << "[DEBUG] LINE " << __LINE__ << endl;
#endif
  while (cin.good()) {
    string line;
    getline(cin, line);
    list<uint8_t> encoded;
    if (line.size() <= 1) {
      break;
    }
    if (filename.size() > 0 && line.size() >= 18 && line.substr(0, 18) == string("#PRAGMA REPLICATE ")) {
      VarEncMap varenc (RIFVar::cmplt0);
      string condstr = line.substr(18);
      RIFCondition condition = parse_condition(condstr);
      encode_condition(encoded, condition, varenc);
      print_encoded(encoded, fout);
      continue;
    } else if (line[0] == '#') {
      continue;
    }
#if DEBUG
    cerr << "[DEBUG] Encoding " << line << endl;
#endif
    encode_rule(encoded, parse_rule(line));
    print_encoded(encoded);
  }
#if DEBUG
  cerr << "[DEBUG] LINE " << __LINE__ << endl;
#endif
  if (filename.size() > 0) {
    fout.close();
  }
#if DEBUG
  cerr << "[DEBUG] LINE " << __LINE__ << endl;
#endif
  if (do_print_constants) {
    print_constants();
  }
#if DEBUG
  cerr << "[DEBUG] LINE " << __LINE__ << endl;
#endif
  return 0;
}
TRACE(TraceableException, "(trace)")

int main(int argc, char **argv) {
  try {
#if DEBUG
    cerr << "[DEBUG] LINE " << __LINE__ << endl;
#endif

    int r = doit(argc, argv);

    // CLEAR OUT THE GLOBAL VARIABLES OR FACE DOOM!
    constenc.clear();
    builtenc.clear();
    execenc.clear();
    funcenc.clear();

    return r;

  } catch (TraceableException &e) {
    cerr << "[ERROR] " << e.what() << endl;
    RETHROWX(e);
  }
}
