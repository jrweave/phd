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

#include "rif/RIFActionBlock.h"

#include <deque>
#include "ptr/APtr.h"
#include "ptr/MPtr.h"

namespace rif {

using namespace ptr;
using namespace std;

RIFActVarBind::RIFActVarBind(const RIFVar &var, const RIFAtomic &frame)
    throw(BadAllocException, TraceableException)
    : var(var) {
  if (frame.getType() != FRAME) {
    THROW(TraceableException,
          "Action variable must be bound to frame or New().");
  }
  TermSet attrs(RIFTerm::cmplt0);
  frame.getAttrs(attrs);
  TermSet::const_iterator it = attrs.begin();
  for (; it != attrs.end(); ++it) {
    TermSet vals(RIFTerm::cmplt0);
    frame.getValues(*it, vals);
    if (vals.count(var) > 0) {
      break;
    }
  }
  if (it == attrs.end()) {
    THROW(TraceableException,
          "Action variable must be slot value in frame.");
  }
  try {
    NEW(this->frame, RIFAtomic, frame);
  } RETHROW_BAD_ALLOC
}

RIFActVarBind::RIFActVarBind(const RIFActVarBind &copy)
    throw(BadAllocException)
    : var(copy.var), frame(NULL) {
  if (copy.frame != NULL) {
    try {
      NEW(this->frame, RIFAtomic, *(copy.frame));
    } RETHROW_BAD_ALLOC
  }
}

RIFActVarBind::~RIFActVarBind() throw() {
  if (this->frame != NULL) {
    DELETE(this->frame);
  }
}

int RIFActVarBind::cmp(const RIFActVarBind &b1, const RIFActVarBind &b2)
    throw() {
  int c = RIFVar::cmp(b1.var, b2.var);
  if (c != 0) {
    return c;
  }
  if (b1.frame == NULL) {
    if (b2.frame == NULL) {
      return 0;
    }
    return -1;
  } else if (b2.frame == NULL) {
    return 1;
  }
  return RIFAtomic::cmp(*(b1.frame), *(b2.frame));
}

RIFActVarBind RIFActVarBind::parse(DPtr<uint8_t> *utf8str)
    throw(BadAllocException, SizeUnknownException,
          InvalidCodepointException, InvalidEncodingException,
          MalformedIRIRefException, TraceableException) {
  if (utf8str == NULL) {
    THROW(TraceableException, "utf8str must not be NULL.");
  }
  if (!utf8str->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  const uint8_t *begin = utf8str->dptr();
  const uint8_t *end = begin + utf8str->size();
  const uint8_t *left_bound = begin;
  const uint8_t *right_bound = end;
  for (; left_bound != end && is_space(*left_bound); ++left_bound) {
    // find first non-space character
  }
  if (left_bound == end || *left_bound != to_ascii('(')) {
    THROW(TraceableException, "Could not find opening parenthesis.");
  }
  for (--right_bound; right_bound != left_bound && is_space(*right_bound);
       --right_bound) {
    // find last non-space character
  }
  if (right_bound == left_bound || *right_bound != to_ascii(')')) {
    THROW(TraceableException, "Ends without closing parenthesis.");
  }
  for (++left_bound; left_bound != right_bound && is_space(*left_bound);
       ++left_bound) {
    // find beginning of action variable
  }
  if (left_bound == right_bound || *left_bound != to_ascii('?')) {
    THROW(TraceableException, "Unable to find action variable.");
  }
  for (--right_bound; right_bound != left_bound && is_space(*right_bound);
       --right_bound) {
    // find end of binding value
  }
  if (right_bound == left_bound) {
    THROW(TraceableException, "Unable to find binding value.");
  }
  ++right_bound;
  const uint8_t *var = left_bound;
  while (var != right_bound) {
    for (++var; var != right_bound && !is_space(*var); ++var) {
      // find end of variable... maybe
    }
    DPtr<uint8_t> *varstr = utf8str->sub(left_bound - begin,
                                         var - left_bound);
    RIFVar v;
    try {
      v = RIFVar::parse(varstr);
      varstr->drop();
    } catch (BadAllocException &e) {
      varstr->drop();
      RETHROW(e, "Unable to parse action variable.");
    } catch (InvalidCodepointException &e) {
      varstr->drop();
      RETHROW(e, "Unable to parse action variable.");
    } catch (InvalidEncodingException &e) {
      varstr->drop();
      RETHROW(e, "Unable to parse action variable.");
    } catch (TraceableException &e) {
      varstr->drop();
      continue;
    }

    const uint8_t *bindval = var;
    for (; bindval != right_bound && is_space(*bindval); ++bindval) {
      // find beginning of binding value
    }
    if (bindval == right_bound) {
      THROW(TraceableException, "Unable to find binding value.");
    }
    if (right_bound - bindval == 5 &&
        ascii_strncmp(bindval, "New()", 5) == 0) {
      return RIFActVarBind(v);
    }
    DPtr<uint8_t> *framestr = utf8str->sub(bindval - begin,
                                           right_bound - bindval);
    try {
      RIFAtomic frame = RIFAtomic::parse(framestr);
      framestr->drop();
      framestr = NULL;
      return RIFActVarBind(v, frame);
    } catch (BadAllocException &e) {
      if (framestr != NULL) {
        framestr->drop();
      }
      RETHROW(e, "Unable to parse binding value.");
    } catch (InvalidCodepointException &e) {
      framestr->drop();
      RETHROW(e, "Unable to parse binding value.");
    } catch (InvalidEncodingException &e) {
      framestr->drop();
      RETHROW(e, "Unable to parse binding value.");
    } catch (MalformedIRIRefException &e) {
      framestr->drop();
      RETHROW(e, "Unable to parse binding value.");
    } catch (TraceableException &e) {
      if (framestr != NULL) {
        framestr->drop();
      }
      RETHROW(e, "Unable to parse binding value.");
    }
  }
  THROW(TraceableException, "Unable to parse action variable binding.");
}

DPtr<uint8_t> *RIFActVarBind::toUTF8String() const throw(BadAllocException) {
  DPtr<uint8_t> *str;
  DPtr<uint8_t> *varstr;
  try {
    varstr = this->var.toUTF8String();
  } JUST_RETHROW(BadAllocException, "Cannot stringify RIFActVarBind.")
  size_t len = 3 + varstr->size();
  if (this->frame == NULL) {
    len += 5;
    try {
      NEW(str, MPtr<uint8_t>, len);
    } catch (bad_alloc &e) {
      varstr->drop();
      THROW(BadAllocException, sizeof(MPtr<uint8_t>));
    } catch (BadAllocException &e) {
      varstr->drop();
      RETHROW(e, "Cannot allocate space for stringifying RIFActVarBind.");
    }
    uint8_t *write_to = str->dptr();
    *write_to = to_ascii('(');
    memcpy(++write_to, varstr->dptr(), varstr->size());
    write_to += varstr->size();
    varstr->drop();
    ascii_strncpy(write_to, " New())", 7);
    return str;
  }
  DPtr<uint8_t> *framestr;
  try {
    framestr = this->frame->toUTF8String();
  } catch (BadAllocException &e) {
    varstr->drop();
    RETHROW(e, "Cannot stringify RIFActVarBind.");
  }
  len += framestr->size();
  try {
    NEW(str, MPtr<uint8_t>, len);
  } catch (bad_alloc &e) {
    varstr->drop();
    framestr->drop();
    THROW(BadAllocException, sizeof(MPtr<uint8_t>));
  } catch (BadAllocException &e) {
    varstr->drop();
    framestr->drop();
    RETHROW(e, "Cannot allocate space for stringifying RIFActVarBind.");
  }
  uint8_t *write_to = str->dptr();
  *write_to = to_ascii('(');
  memcpy(++write_to, varstr->dptr(), varstr->size());
  write_to += varstr->size();
  varstr->drop();
  *write_to = to_ascii(' ');
  memcpy(++write_to, framestr->dptr(), framestr->size());
  write_to += framestr->size();
  framestr->drop();
  *write_to = to_ascii(')');
  return str;
}

RIFActVarBind &RIFActVarBind::normalize() THROWS(BadAllocException) {
  this->var.normalize();
  if (this->frame != NULL) {
    this->frame->normalize();
  }
}
TRACE(BadAllocException, "Unable to normalize RIFActVarBind.")

void RIFActVarBind::getVars(VarSet &vars, const bool include_act_var) const
    throw() {
  if (include_act_var) {
    vars.insert(this->var);
  }
  if (this->frame != NULL) {
    this->frame->getVars(vars);
  }
}

RIFActVarBind &RIFActVarBind::operator=(const RIFActVarBind &rhs)
    throw(BadAllocException) {
  RIFAtomic *f = NULL;
  if (rhs.frame != NULL) {
    try {
      NEW(f, RIFAtomic, *(rhs.frame));
    } RETHROW_BAD_ALLOC
  }
  this->var = rhs.var;
  if (this->frame != NULL) {
    DELETE(this->frame);
  }
  this->frame = f;
  return *this;
}



///// RIFActionBlock /////

RIFActionBlock::RIFActionBlock() THROWS(BadAllocException) {
  NEW(this->actions, APtr<RIFAction>, 1);
}
TRACE(BadAllocException, "(trace)")

RIFActionBlock::RIFActionBlock(DPtr<RIFAction> *acts)
    throw(BaseException<void*>, SizeUnknownException, TraceableException)
    : actions(acts) {
  if (acts == NULL) {
    THROW(BaseException<void*>, NULL, "acts must not be NULL.");
  }
  if (!acts->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  if (acts->size() < 1) {
    THROW(TraceableException,
          "RIFActionBlock must contain at least one action.");
  }
  RIFAction *act = acts->dptr();
  RIFAction *end = act + acts->size();
  for (; act != end; ++act) {
    if (act->getType() == ASSERT_FACT) {
      RIFAtomic atom = act->getTargetAtomic();
      if (atom.getType() == MEMBERSHIP) {
        THROW(TraceableException,
              "Cannot assert membership atom without action variable.");
      }
    }
  }
  this->actions->hold();
}

RIFActionBlock::RIFActionBlock(DPtr<RIFAction> *acts, const ActVarMap &actvars)
    throw(BaseException<void*>, SizeUnknownException, TraceableException)
    : actions(acts), actvars(actvars) {
  if (acts == NULL) {
    THROW(BaseException<void*>, NULL, "acts must not be NULL.");
  }
  if (!acts->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  if (acts->size() < 1) {
    THROW(TraceableException,
          "RIFActionBlock must contain at least one action.");
  }
  ActVarMap::const_iterator it = actvars.begin();
  for (; it != actvars.end(); ++it) {
    if (!it->second.getActVar().equals(it->first)) {
      THROW(TraceableException, "Invalid ActVarMap.");
    }
  }
  RIFAction *act = acts->dptr();
  RIFAction *end = act + acts->size();
  for (; act != end; ++act) {
    if (act->getType() == ASSERT_FACT) {
      RIFAtomic atom = act->getTargetAtomic();
      if (atom.getType() == MEMBERSHIP) {
        RIFTerm obj = atom.getObject();
        if (obj.getType() != VARIABLE || actvars.count(obj.getVar()) < 1) {
          THROW(TraceableException,
                "Asserted membership atom must have action variable object.");
        }
      }
    }
  }
  this->actions->hold();
}

RIFActionBlock::RIFActionBlock(const RIFActionBlock &copy) throw()
    : actions(copy.actions), actvars(copy.actvars) {
  this->actions->hold();
}

RIFActionBlock::~RIFActionBlock() throw() {
  this->actions->drop();
}

int RIFActionBlock::cmp(const RIFActionBlock &a1, const RIFActionBlock &a2)
    throw() {
  if (a1.actions->size() < a2.actions->size()) {
    return -1;
  } else if (a1.actions->size() > a2.actions->size()) {
    return 1;
  }
  if (a1.actvars.size() < a2.actvars.size()) {
    return -1;
  } else if (a1.actvars.size() > a2.actvars.size()) {
    return 1;
  }
  ActVarMap::const_iterator it1 = a1.actvars.begin();
  ActVarMap::const_iterator it2 = a2.actvars.begin();
  for (; it1 != a1.actvars.end(); ++it1) {
    int c = RIFActVarBind::cmp(it1->second, it2->second);
    if (c != 0) {
      return c;
    }
    ++it2;
  }
  RIFAction *act1 = a1.actions->dptr();
  RIFAction *end = act1 + a1.actions->size();
  RIFAction *act2 = a2.actions->dptr();
  for (; act1 != end; ++act1) {
    int c = RIFAction::cmp(*act1, *act2);
    if (c != 0) {
      return c;
    }
    ++act2;
  }
  return 0;
}

RIFActionBlock RIFActionBlock::parse(DPtr<uint8_t> *utf8str)
    throw(BadAllocException, SizeUnknownException,
          InvalidCodepointException, InvalidEncodingException,
          MalformedIRIRefException, TraceableException) {
  if (utf8str == NULL) {
    THROW(TraceableException, "utf8str must not be NULL.");
  }
  if (!utf8str->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  const uint8_t *begin = utf8str->dptr();
  const uint8_t *end = begin + utf8str->size();
  const uint8_t *mark = begin;
  for (; mark != end && is_space(*mark); ++mark) {
    // find beginning of action block
  }
  if (end - mark < 4) {
    THROW(TraceableException, "Could not find action block.");
  }
  if (ascii_strncmp(mark, "Do", 2) != 0) {
    THROW(TraceableException, "Action block must start with \"Do\".");
  }
  mark += 2;
  for (; mark != end && is_space(*mark); ++mark) {
    // search for left parenthesis
  }
  if (mark == end || *mark != to_ascii('(')) {
    THROW(TraceableException, "Could not find opening parenthesis.");
  }
  for (--end; end != mark && is_space(*end); --end) {
    // find right parenthesis
  }
  if (end == mark || *end != to_ascii(')')) {
    THROW(TraceableException, "Could not find closing parenthesis.");
  }
  for (--end; end != mark && is_space(*end); --end) {
    // find end of last action
  }
  ++end;
  for (++mark; mark != end && is_space(*mark); ++mark) {
    // find first binding or action
  }
  if (mark == end) {
    THROW(TraceableException, "Action block is (illegaly) empty.");
  }
  deque<RIFAction> actions;
  ActVarMap bindings(RIFVar::cmplt0);
  bool find_bindings = (*mark == to_ascii('('));
  const uint8_t *bound = mark;
  while (bound != end) {
    for (; bound != end && *bound != to_ascii(')'); ++bound) {
      // find end of action block binding or action... maybe
    }
    if (bound != end) {
      ++bound;
    }
    DPtr<uint8_t> *str = utf8str->sub(mark - begin, bound - mark);
    if (find_bindings) {
      try {
        RIFActVarBind bind = RIFActVarBind::parse(str);
        str->drop();
        str = NULL;
        pair<ActVarMap::iterator, bool> p = bindings.insert(
            pair<RIFVar, RIFActVarBind>(bind.getActVar(), bind));
        if (!p.second) {
          THROW(TraceableException,
                "Cannot have multiple bindings for same action variable.");
        }
      } catch (BadAllocException &e) {
        str->drop();
        RETHROW(e, "Cannot parse action variable binding.");
      } catch (InvalidCodepointException &e) {
        str->drop();
        RETHROW(e, "Cannot parse action variable binding.");
      } catch (InvalidEncodingException &e) {
        str->drop();
        RETHROW(e, "Cannot parse action variable binding.");
      } catch (MalformedIRIRefException &e) {
        str->drop();
        continue;
      } catch (TraceableException &e) {
        if (str == NULL) {
          RETHROW(e, "Malformed action block.");
        }
        str->drop();
        continue;
      }
    } else {
      try {
        RIFAction act = RIFAction::parse(str);
        str->drop();
        actions.push_back(act);
      } catch (BadAllocException &e) {
        str->drop();
        RETHROW(e, "Cannot parse action.");
      } catch (InvalidCodepointException &e) {
        str->drop();
        RETHROW(e, "Cannot parse action.");
      } catch (InvalidEncodingException &e) {
        str->drop();
        RETHROW(e, "Cannot parse action.");
      } catch (MalformedIRIRefException &e) {
        str->drop();
        continue;
      } catch (TraceableException &e) {
        str->drop();
        continue;
      }
    }
    for (mark = bound; mark != end && is_space(*mark); ++mark) {
      // find beginning of next action block binding or action... maybe
    }
    bound = mark;
    find_bindings = find_bindings && (*mark == to_ascii('('));
  }
  DPtr<RIFAction> *pact;
  try {
    NEW(pact, APtr<RIFAction>, actions.size());
  } RETHROW_BAD_ALLOC
  copy(actions.begin(), actions.end(), pact->dptr());
  try {
    RIFActionBlock block(pact, bindings);
    pact->drop();
    return block;
  } catch (TraceableException &e) {
    pact->drop();
    RETHROW(e, "Cannot parse action block.");
  }
}

DPtr<uint8_t> *RIFActionBlock::toUTF8String() const throw(BadAllocException) {
  deque<void*> subs;
  try {
    size_t len = 3 + this->actions->size() + this->actvars.size();
    ActVarMap::const_iterator it = this->actvars.begin();
    for (; it != this->actvars.end(); ++it) {
      DPtr<uint8_t> *s = it->second.toUTF8String();
      subs.push_back((void*)s);
      len += s->size();
    }
    RIFAction *act = this->actions->dptr();
    RIFAction *end = act + this->actions->size();
    for (; act != end; ++act) {
      DPtr<uint8_t> *s = act->toUTF8String();
      subs.push_back((void*)s);
      len += s->size();
    }
    DPtr<uint8_t> *str;
    try {
      NEW(str, MPtr<uint8_t>, len);
    } RETHROW_BAD_ALLOC
    uint8_t *write_to = str->dptr();
    ascii_strncpy(write_to, "Do(", 3);
    write_to += 3;
    deque<void*>::iterator pit = subs.begin();
    for (; pit != subs.end(); ++pit) {
      if (pit != subs.begin()) {
        *write_to = to_ascii(' ');
        ++write_to;
      }
      DPtr<uint8_t> *p = (DPtr<uint8_t>*) (*pit);
      memcpy(write_to, p->dptr(), p->size());
      write_to += p->size();
      p->drop();
      *pit = NULL;
    }
    *write_to = to_ascii(')');
    return str;
  } catch (BadAllocException &e) {
    deque<void*>::iterator it = subs.begin();
    for (; it != subs.end(); ++it) {
      if (*it != NULL) {
        ((DPtr<uint8_t>*)(*it))->drop();
      }
    }
    RETHROW(e, "Unable to stringify action block.");
  }
}

RIFActionBlock &RIFActionBlock::normalize() THROWS(BadAllocException) {
  ActVarMap newactvars(RIFVar::cmplt0);
  ActVarMap::iterator it = this->actvars.begin();
  for (; it != this->actvars.end(); ++it) {
    RIFActVarBind bind = it->second;
    bind.normalize();
    newactvars.insert(pair<RIFVar, RIFActVarBind>(bind.getActVar(), bind));
  }
  this->actvars.swap(newactvars);
  if (!this->actions->alone()) {
    this->actions = this->actions->stand();
  }
  RIFAction *act = this->actions->dptr();
  RIFAction *end = act + this->actions->size();
  for (; act != end; ++act) {
    act->normalize();
  }
  return *this;
}
TRACE(BadAllocException, "Unable to normalize action block.")

bool RIFActionBlock::isGround(const bool include_act_var) const throw() {
  ActVarMap::const_iterator it = this->actvars.begin();
  for (; it != this->actvars.end(); ++it) {
    if (!it->second.isGround(include_act_var)) {
      return false;
    }
  }
  RIFAction *act = this->actions->dptr();
  RIFAction *end = act + this->actions->size();
  for (; act != end; ++act) {
    if (!act->isGround()) {
      return false;
    }
  }
  return true;
}

void RIFActionBlock::getVars(VarSet &vars, const bool include_act_var) const
    throw() {
  ActVarMap::const_iterator it = this->actvars.begin();
  for (; it != this->actvars.end(); ++it) {
    it->second.getVars(vars, include_act_var);
  }
  RIFAction *act = this->actions->dptr();
  RIFAction *end = act + this->actions->size();
  for (; act != end; ++act) {
    act->getVars(vars);
  }
}

bool RIFActionBlock::getActVars(VarSet *vars) const throw() {
  if (this->actvars.empty()) {
    return false;
  }
  if (vars != NULL) {
    ActVarMap::const_iterator it = this->actvars.begin();
    for (; it != this->actvars.end(); ++it) {
      vars->insert(it->first);
    }
  }
  return true;
}

RIFActVarBind RIFActionBlock::getBinding(const RIFVar &var) const
    throw(TraceableException) {
  ActVarMap::const_iterator it = this->actvars.find(var);
  if (it == this->actvars.end()) {
    THROW(TraceableException, "No such action variable.");
  }
  return it->second;
}

RIFActionBlock &RIFActionBlock::operator=(const RIFActionBlock &rhs)
    THROWS(BadAllocException) {
  this->actvars.clear();
  this->actvars.insert(rhs.actvars.begin(), rhs.actvars.end());
  rhs.actions->hold();
  this->actions->drop();
  this->actions = rhs.actions;
  return *this;
}
TRACE(BadAllocException, "Failed assignment to RIFActionBlock.")

}
