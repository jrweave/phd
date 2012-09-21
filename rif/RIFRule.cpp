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

#include "rif/RIFRule.h"

namespace rif {

using namespace ptr;
using namespace std;

int RIFRule::cmp(const RIFRule &r1, const RIFRule &r2) throw() {
  int c = RIFActionBlock::cmp(r1.actionblock, r2.actionblock);
  if (c != 0) {
    return c;
  }
  return RIFCondition::cmp(r1.condition, r2.condition);
}

RIFRule RIFRule::parse(DPtr<uint8_t> *utf8str)
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
  const uint8_t *left = begin;
  for (; left != end && is_space(*left); ++left) {
    // find beginning of rule
  }
  if (end - left < 3 || ascii_strncmp(left, "If", 2) != 0) {
    THROW(TraceableException, "Rule must start with \"If\".");
  }
  left += 2;
  if (!is_space(*left)) {
    THROW(TraceableException, "Some space must follow \"If\".");
  }
  for (; left != end && is_space(*left); ++left) {
    // find beginning of condition
  }
  if (end - left < 6) {
    THROW(TraceableException, "No condition formula found.");
  }
  const uint8_t *right = end - 6;
  while (right != left) {
    for (; right != left &&
           (!is_space(*right) ||
            ascii_strncmp(right + 1, "Then", 4) != 0 ||
            !is_space(*(right+5))); --right) {
      // find a "Then" surrounded by space
    }
    const uint8_t *tmp = right;
    for (; tmp != left && is_space(*tmp); --tmp) {
      // find potential end of condition
    }

    // TODO delete when no longer needed
    #if 0
    const uint8_t *x = left;
    cerr << "CHECK1: ";
    for (; x != tmp + 1; ++x) {
      cerr << to_lchar(*x);
    }
    cerr << endl;
    #endif

    DPtr<uint8_t> *str = utf8str->sub(left - begin,
                                      tmp - left + 1);
    RIFCondition condition;
    try {
      condition = RIFCondition::parse(str);
      str->drop();
    } catch (BadAllocException &e) {
      str->drop();
      RETHROW(e, "Unable to parse rule.");
    } catch (InvalidCodepointException &e) {
      str->drop();
      RETHROW(e, "Unable to parse rule.");
    } catch (InvalidEncodingException &e) {
      str->drop();
      RETHROW(e, "Unable to parse rule.");
    } catch (MalformedIRIRefException &e) {
      str->drop();
      RETHROW(e, "Unable to parse rule.");
    } catch (TraceableException &e) {
      str->drop();
      continue;
    }

    // TODO delete when no longer needed
    #if 0
    x = right + 6;
    cerr << "CHECK2: ";
    for (; x != end; ++x) {
      cerr << to_lchar(*x);
    }
    cerr << endl;
    #endif

    str = utf8str->sub(right + 6 - begin,
                       end - right - 6);
    RIFActionBlock actionblock;
    try {
      actionblock = RIFActionBlock::parse(str);
      str->drop();
    } catch (BadAllocException &e) {
      str->drop();
      RETHROW(e, "Unable to parse rule.");
    } catch (InvalidCodepointException &e) {
      str->drop();
      RETHROW(e, "Unable to parse rule.");
    } catch (InvalidEncodingException &e) {
      str->drop();
      RETHROW(e, "Unable to parse rule.");
    } catch (MalformedIRIRefException &e) {
      str->drop();
      RETHROW(e, "Unable to parse rule.");
    } catch (TraceableException &e) {
      str->drop();
      continue;
    }
    try {
      return RIFRule(condition, actionblock);
    } JUST_RETHROW(BadAllocException, "Unable to parse rule.")
  }
  THROW(TraceableException, "Unable to parse rule.");
}

DPtr<uint8_t> *RIFRule::toUTF8String() const throw(BadAllocException) {
  size_t len = 9;
  DPtr<uint8_t> *condstr, *actstr;
  try {
    condstr = this->condition.toUTF8String();
  } JUST_RETHROW(BadAllocException, "Unable to stringify condition.")
  try {
    actstr = this->actionblock.toUTF8String();
  } catch (BadAllocException &e) {
    condstr->drop();
    RETHROW(e, "Unable to stringify action block.");
  }
  len += condstr->size() + actstr->size();
  DPtr<uint8_t> *str;
  try {
    NEW(str, MPtr<uint8_t>, len);
  } catch (bad_alloc &e) {
    condstr->drop();
    actstr->drop();
    THROW(BadAllocException, sizeof(MPtr<uint8_t>));
  } catch (BadAllocException &e) {
    condstr->drop();
    actstr->drop();
    RETHROW(e, "Unable to stringify rule.");
  }
  uint8_t *write_to = str->dptr();
  ascii_strncpy(write_to, "If ", 3);
  write_to += 3;
  memcpy(write_to, condstr->dptr(), condstr->size());
  write_to += condstr->size();
  condstr->drop();
  ascii_strncpy(write_to, " Then ", 6);
  write_to += 6;
  memcpy(write_to, actstr->dptr(), actstr->size());
  actstr->drop();
  return str;
}

RIFRule &RIFRule::operator=(const RIFRule &rhs) THROWS(BadAllocException) {
  this->condition = rhs.condition;
  this->actionblock = rhs.actionblock;
  return *this;
}
TRACE(BadAllocException, "Failed RIFRule assignment; object corrupted.");

}
