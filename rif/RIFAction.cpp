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

#include "rif/RIFAction.h"

namespace rif {

using namespace ptr;
using namespace std;

RIFAction::RIFAction() throw()
    : target(NULL), type(EXECUTE) {
  // do nothing
}

RIFAction::RIFAction(const enum RIFActType type, const RIFAtomic &target)
    throw(BaseException<enum RIFActType>, BadAllocException)
    : type(type) {
  switch (type) {
  case ASSERT_FACT: {
    enum RIFAtomicType atomic_type = target.getType();
    if (atomic_type != ATOM && atomic_type != FRAME &&
        atomic_type != MEMBERSHIP) {
      THROW(BaseException<enum RIFActType>, type,
        "target for ASSERT_FACT action must be ATOM, FRAME, or MEMBERSHIP.");
    }
    try {
      NEW(this->target, RIFAtomic, target);
    } RETHROW_BAD_ALLOC
    break;
  }
  case RETRACT_FACT: {
    enum RIFAtomicType atomic_type = target.getType();
    if (atomic_type != ATOM && atomic_type != FRAME) {
      THROW(BaseException<enum RIFActType>, type,
        "target for RETRACT_FACT action must be ATOM or FRAME.");
    }
    try {
      NEW(this->target, RIFAtomic, target);
    } RETHROW_BAD_ALLOC
    break;
  }
  case RETRACT_SLOTS: {
    THROW(BaseException<enum RIFActType>, type,
        "target for RETRACT_SLOTS action must be a pair<RIFTerm, RIFTerm>.");
  }
  case RETRACT_OBJECT: {
    THROW(BaseException<enum RIFActType>, type,
        "target for RETRACT_OBJECT action must be a RIFTerm.");
  }
  case EXECUTE: {
    if (target.getType() != ATOM) {
      THROW(BaseException<enum RIFActType>, type,
          "target for EXECUTE action must be ATOM.");
    }
    try {
      NEW(this->target, RIFAtomic, target);
    } RETHROW_BAD_ALLOC
    break;
  }
  case MODIFY: {
    if (target.getType() != FRAME) {
      THROW(BaseException<enum RIFActType>, type,
          "target for MODIFY action must be FRAME.");
    }
    try {
      NEW(this->target, RIFAtomic, target);
    } RETHROW_BAD_ALLOC
    break;
  }
  default: {
    // this should never happen
    THROW(BaseException<enum RIFActType>, type,
      "Unhandled type.  This should never happen.");
  }
  }
}

RIFAction::RIFAction(const RIFAction &copy) throw(BadAllocException)
    : type(copy.type), target(NULL) {
  switch (copy.type) {
  case RETRACT_SLOTS: {
    pair<RIFTerm, RIFTerm> *p = (pair<RIFTerm, RIFTerm>*) copy.target;
    try {
      NEW(this->target, WHOLE(pair<RIFTerm, RIFTerm>), p->first, p->second);
    } RETHROW_BAD_ALLOC
    break;
  }
  case RETRACT_OBJECT: {
    RIFTerm *t = (RIFTerm*) copy.target;
    try {
      NEW(this->target, RIFTerm, *t);
    } RETHROW_BAD_ALLOC
    break;
  }
  default: {
    if (copy.target != NULL) {
      RIFAtomic *a = (RIFAtomic*) copy.target;
      try {
        NEW(this->target, RIFAtomic, *a);
      } RETHROW_BAD_ALLOC
    }
    break;
  }
  }
}

RIFAction::~RIFAction() throw() {
  switch (this->type) {
  case RETRACT_SLOTS:
    DELETE((pair<RIFTerm, RIFTerm>*) this->target);
    break;
  case RETRACT_OBJECT:
    DELETE((RIFTerm*) this->target);
    break;
  default:
    if (this->target != NULL) {
      DELETE((RIFAtomic*) this->target);
    }
    break;
  }
}

int RIFAction::cmp(const RIFAction &a1, const RIFAction &a2) throw() {
  int typen1 = (int)a1.type;
  int typen2 = (int)a2.type;
  int diff = typen1 - typen2;
  if (diff != 0) {
    return diff;
  }
  switch (a1.type) {
  case RETRACT_SLOTS: {
    pair<RIFTerm, RIFTerm> *p1 = (pair<RIFTerm, RIFTerm>*) a1.target;
    pair<RIFTerm, RIFTerm> *p2 = (pair<RIFTerm, RIFTerm>*) a2.target;
    int c = RIFTerm::cmp(p1->first, p2->first);
    if (c != 0) {
      return c;
    }
    return RIFTerm::cmp(p1->second, p2->second);
  }
  case RETRACT_OBJECT: {
    RIFTerm *t1 = (RIFTerm*) a1.target;
    RIFTerm *t2 = (RIFTerm*) a2.target;
    return RIFTerm::cmp(*t1, *t2);
  }
  default: {
    if (a1.target == NULL) {
      if (a2.target == NULL) {
        return 0;
      }
      return  -1;
    } else if (a2.target == NULL) {
      return 1;
    }
    RIFAtomic *at1 = (RIFAtomic*) a1.target;
    RIFAtomic *at2 = (RIFAtomic*) a2.target;
    return RIFAtomic::cmp(*at1, *at2);
  }
  }
}

RIFAction RIFAction::parse(DPtr<uint8_t> *utf8str)
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
  const uint8_t *beginkw = begin;
  for (; beginkw != end && is_space(*beginkw); ++beginkw) {
    // find beginning of keyword
  }
  if (beginkw == end) {
    THROW(TraceableException, "No keyword found when parsing RIFAction.");
  }
  const uint8_t *endkw = beginkw;
  for (; endkw != end && is_alnum(*endkw); ++endkw) {
    // find end of keyword
  }
  
  enum RIFActType type;
  if (endkw - beginkw == 6) {
    if (ascii_strncmp(beginkw, "Assert", 6) == 0) {
      type = ASSERT_FACT;
    } else if (ascii_strncmp(beginkw, "Modify", 6) == 0) {
      type = MODIFY;
    } else {
      THROW(TraceableException, "Unrecognized RIFAction type.");
    }
  } else if (endkw - beginkw == 7) {
    if (ascii_strncmp(beginkw, "Retract", 7) == 0) {
      type = RETRACT_FACT; // validate later
    } else if (ascii_strncmp(beginkw, "Execute", 7) == 0) {
      type = EXECUTE;
    } else {
      THROW(TraceableException, "Unrecognized RIFAction type.");
    }
  } else {
    THROW(TraceableException, "Unrecognized RIFAction type.");
  }

  const uint8_t *left_bound = endkw;
  for (; left_bound != end && *left_bound != to_ascii('('); ++left_bound) {
    // find left paren enclosing target
  }
  if (left_bound == end) {
    THROW(TraceableException,
          "Could not find left paren when parsing RIFAction.");
  }
  const uint8_t *right_bound = end;
  for (--right_bound; right_bound != left_bound &&
                      *right_bound != to_ascii(')'); --right_bound) {
    // find right paren enclosing target
  }
  if (right_bound == left_bound) {
    THROW(TraceableException,
          "Could not find right paren when parsing RIFAction.");
  }
  for (++left_bound; left_bound != right_bound && is_space(*left_bound);
       ++left_bound) {
    // find left bound of target
  }
  if (left_bound == right_bound) {
    if (type == EXECUTE) {
      return RIFAction();
    }
    THROW(TraceableException, "No target specified for RIFAction.");
  }
  for (--right_bound; right_bound != left_bound && is_space(*right_bound);
       --right_bound) {
    // find right bound of target
  }
  ++right_bound;
  DPtr<uint8_t> *targetstr = utf8str->sub(left_bound - begin,
                                          right_bound - left_bound);
  try {
    RIFAtomic atom = RIFAtomic::parse(targetstr);
    RIFAction act (type, atom);
    targetstr->drop();
    return act;
  } catch (BadAllocException &e) {
    targetstr->drop();
    RETHROW(e, "Unable to parse target.");
  } catch (InvalidCodepointException &e) {
    targetstr->drop();
    RETHROW(e, "Unable to parse target.");
  } catch (InvalidEncodingException &e) {
    targetstr->drop();
    RETHROW(e, "Unable to parse target.");
  } catch (MalformedIRIRefException &e) {
    targetstr->drop();
    RETHROW(e, "Unable to parse target.");
  } catch (BaseException<enum RIFActType> &e) {
    targetstr->drop();
    RETHROW(e, "Unable to parse target.");
  } catch (TraceableException &e) {
    if (type != RETRACT_FACT) {
      targetstr->drop();
      RETHROW(e, "Unable to parse target.");
    }
  }
  try {
    RIFTerm term = RIFTerm::parse(targetstr);
    RIFAction act (term);
    targetstr->drop();
    return act;
  } catch (BadAllocException &e) {
    targetstr->drop();
    RETHROW(e, "Unable to parse target.");
  } catch (InvalidCodepointException &e) {
    targetstr->drop();
    RETHROW(e, "Unable to parse target.");
  } catch (InvalidEncodingException &e) {
    targetstr->drop();
    RETHROW(e, "Unable to parse target.");
  } catch (MalformedIRIRefException &e) {
    targetstr->drop();
    RETHROW(e, "Unable to parse target.");
  } catch (TraceableException &e) {
    // ignore and check if RETRACT_SLOTS
  }
  RIFTerm obj, attr;
  begin = targetstr->dptr();
  end = begin + targetstr->size();
  while (begin != end) {
    for (; begin != end && is_space(*begin); ++begin) {
      // creep forward to first non-space
    }
    for (++begin; begin != end && !is_space(*begin); ++begin) {
      // search for space between terms
    }
    if (begin == end) {
      targetstr->drop();
      THROW(TraceableException, "Invalid RETRACT target.");
    }
    DPtr<uint8_t> *str = targetstr->sub(0, begin - targetstr->dptr());
    try {
      obj = RIFTerm::parse(str);
      str->drop();
    } catch (TraceableException &e) {
      str->drop();
      continue;
    }
    for (++begin; begin != end && is_space(*begin); ++begin) {
      // search for second term
    }
    str = targetstr->sub(begin - targetstr->dptr(), end - begin);
    try {
      attr = RIFTerm::parse(str);
      str->drop();
    } catch (TraceableException &e) {
      str->drop();
      continue;
    }
    try {
      targetstr->drop();
      return RIFAction(obj, attr);
    } RETHROW_BAD_ALLOC
  }
  targetstr->drop();
  THROW(TraceableException, "Invalid RETRACT target.");
}

DPtr<uint8_t> *RIFAction::toUTF8String() const throw(BadAllocException) {
  size_t len = 2;
  switch (this->type) {
  case ASSERT_FACT:
  case MODIFY:
    len += 6;
    break;
  case RETRACT_FACT:
  case RETRACT_SLOTS:
  case RETRACT_OBJECT:
  case EXECUTE:
    len += 7;
    break;
  default:
    // This should never happen, but just in case, do something.
    THROW(BadAllocException, 0); // nonsensical, but better than continuing
  }
  DPtr<uint8_t> *targetstr1 = NULL;
  DPtr<uint8_t> *targetstr2 = NULL;
  switch (this->type) {
  case RETRACT_SLOTS: {
    pair<RIFTerm, RIFTerm> *p = (pair<RIFTerm, RIFTerm>*) this->target;
    try {
      targetstr1 = p->first.toUTF8String();
    } JUST_RETHROW(BadAllocException, "Problem stringifying object term.")
    try {
      targetstr2 = p->second.toUTF8String();
    } catch (BadAllocException &e) {
      targetstr1->drop();
      RETHROW(e, "Problem stringifying attribute term.");
    }
    break;
  }
  case RETRACT_OBJECT: {
    RIFTerm *t = (RIFTerm*) this->target;
    try {
      targetstr1 = t->toUTF8String();
    } JUST_RETHROW(BadAllocException, "Problem stringifying term target.")
    break;
  }
  default: {
    if (this->target != NULL) {
      RIFAtomic *a = (RIFAtomic*) this->target;
      try {
        targetstr1 = a->toUTF8String();
      } JUST_RETHROW(BadAllocException, "Problem stringifying atomic target.")
    }
    break;
  }
  }
  if (targetstr1 != NULL) {
    len += targetstr1->size();
  }
  if (targetstr2 != NULL) {
    len += 1 + targetstr2->size();
  }
  DPtr<uint8_t> *str = NULL;
  try {
    NEW(str, MPtr<uint8_t>, len);
  } catch (bad_alloc &e) {
    if (targetstr1 != NULL) {
      targetstr1->drop();
    }
    if (targetstr2 != NULL) {
      targetstr2->drop();
    }
    THROW(BadAllocException, sizeof(MPtr<uint8_t>));
  } catch (BadAllocException &e) {
    if (targetstr1 != NULL) {
      targetstr1->drop();
    }
    if (targetstr2 != NULL) {
      targetstr2->drop();
    }
    RETHROW(e, "Cannot stringify RIFAction.");
  }
  uint8_t *mark = str->dptr();
  switch (this->type) {
  case ASSERT_FACT:
    ascii_strcpy(mark, "Assert(");
    mark += 7;
    break;
  case RETRACT_FACT:
  case RETRACT_SLOTS:
  case RETRACT_OBJECT:
    ascii_strcpy(mark, "Retract(");
    mark += 8;
    break;
  case EXECUTE:
    ascii_strcpy(mark, "Execute(");
    mark += 8;
    break;
  case MODIFY:
    ascii_strcpy(mark, "Modify(");
    mark += 7;
    break;
  default:
    // This should never happen.
    THROW(BadAllocException, 0);
  }
  if (targetstr1 != NULL) {
    memcpy(mark, targetstr1->dptr(), targetstr1->size() * sizeof(uint8_t));
    mark += targetstr1->size();
    targetstr1->drop();
  }
  if (targetstr2 != NULL) {
    *mark = to_ascii(' ');
    ++mark;
    memcpy(mark, targetstr2->dptr(), targetstr2->size() * sizeof(uint8_t));
    mark += targetstr2->size();
    targetstr2->drop();
  }
  *mark = to_ascii(')');
  return str;
}

RIFAction &RIFAction::normalize() THROWS(BadAllocException) {
  switch (this->type) {
  case RETRACT_SLOTS: {
    pair<RIFTerm, RIFTerm> *p = (pair<RIFTerm, RIFTerm> *) this->target;
    p->first.normalize();
    p->second.normalize();
    break;
  }
  case RETRACT_OBJECT: {
    RIFTerm *t = (RIFTerm *) this->target;
    t->normalize();
    break;
  }
  default: {
    if (this->target != NULL) {
      RIFAtomic *a = (RIFAtomic *) this->target;
      a->normalize();
    }
    break;
  }
  }
  return *this;
}
TRACE(BadAllocException, "(trace) normalize")

bool RIFAction::isGround() const throw() {
  switch (this->type) {
  case RETRACT_SLOTS: {
    pair<RIFTerm, RIFTerm> *p = (pair<RIFTerm, RIFTerm> *) this->target;
    return p->first.isGround() && p->second.isGround();
  }
  case RETRACT_OBJECT: {
    RIFTerm *t = (RIFTerm *) this->target;
    return t->isGround();
  }
  default: {
    if (this->target == NULL) {
      return true;
    }
    RIFAtomic *a = (RIFAtomic *) this->target;
    return a->isGround();
  }
  }
}

void RIFAction::getVars(VarSet &vars) const throw() {
  switch (this->type) {
  case RETRACT_SLOTS: {
    pair<RIFTerm, RIFTerm> *p = (pair<RIFTerm, RIFTerm> *) this->target;
    p->first.getVars(vars);
    p->second.getVars(vars);
    return;
  }
  case RETRACT_OBJECT: {
    RIFTerm *t = (RIFTerm *) this->target;
    t->getVars(vars);
    return;
  }
  default: {
    if (this->target != NULL) {
      RIFAtomic *a = (RIFAtomic *) this->target;
      a->getVars(vars);
    }
    return;
  }
  }
}

RIFAtomic RIFAction::getTargetAtomic() const
    throw(BaseException<enum RIFActType>) {
  switch (this->type) {
  case ASSERT_FACT:
  case RETRACT_FACT:
  case EXECUTE:
  case MODIFY:
    if (this->target != NULL) {
      return *((RIFAtomic*) this->target);
    }
  }
  THROW(BaseException<enum RIFActType>, this->type,
      "Target is not an atomic formula.");
}

pair<RIFTerm, RIFTerm> RIFAction::getTargetTermPair() const
    throw(BaseException<enum RIFActType>) {
  if (this->type == RETRACT_SLOTS) {
    return *((pair<RIFTerm, RIFTerm>*) this->target);
  }
  THROW(BaseException<enum RIFActType>, this->type,
      "Target is not a pair of terms.");
}

RIFTerm RIFAction::getTargetTerm() const
    throw(BaseException<enum RIFActType>) {
  if (this->type == RETRACT_OBJECT) {
    return *((RIFTerm*) this->target);
  }
  THROW(BaseException<enum RIFActType>, this->type,
      "Target is not a term.");
}

RIFAction &RIFAction::operator=(const RIFAction &rhs)
    throw(BadAllocException) {
  void *new_target = NULL;
  switch (rhs.type) {
  case RETRACT_SLOTS: {
    pair<RIFTerm, RIFTerm> *p = (pair<RIFTerm, RIFTerm>*) rhs.target;
    try {
      NEW(new_target, WHOLE(pair<RIFTerm, RIFTerm>), p->first, p->second);
    } RETHROW_BAD_ALLOC
    break;
  }
  case RETRACT_OBJECT: {
    RIFTerm *t = (RIFTerm*) rhs.target;
    try {
      NEW(new_target, RIFTerm, *t);
    } RETHROW_BAD_ALLOC
    break;
  }
  default: {
    if (rhs.target != NULL) {
      RIFAtomic *a = (RIFAtomic*) rhs.target;
      try {
        NEW(new_target, RIFAtomic, *a);
      } RETHROW_BAD_ALLOC
    }
    break;
  }
  }
  switch (this->type) {
  case RETRACT_SLOTS:
    DELETE((pair<RIFTerm, RIFTerm>*) this->target);
    break;
  case RETRACT_OBJECT:
    DELETE((RIFTerm*) this->target);
    break;
  default:
    if (this->target != NULL) {
      DELETE((RIFAtomic*) this->target);
    }
    break;
  }
  this->type = rhs.type;
  this->target = new_target;
  return *this;
}

}
