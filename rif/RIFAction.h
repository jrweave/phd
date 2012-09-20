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

#ifndef __RIF__RIFACTION_H__
#define __RIF__RIFACTION_H__

#include "rif/RIFAtomic.h"

namespace rif {

using namespace ptr;
using namespace std;

enum RIFActType {
  ASSERT_FACT = 0,
  RETRACT_FACT = 1,
  RETRACT_SLOTS = 2,
  RETRACT_OBJECT = 3,
  EXECUTE = 4,
  MODIFY = 5
};

class RIFAction {
private:
  void *target;
  // ASSERT_FACT: RIFAtomic* (ATOM, FRAME, MEMBERSHIP)
  // RETRACT_FACT: RIFAtomic* (ATOM, FRAME)
  // RETRACT_SLOTS: pair<RIFTerm,RIFTerm>*
  // RETRACT_OBJECT: RIFTerm*
  // EXECUTE: RIFAtomic* (ATOM) or NULL (no-op)
  // MODIFY: RIFAtomic* (FRAME)
  enum RIFActType type;
public:
  RIFAction() throw();
  RIFAction(const enum RIFActType type, const RIFAtomic &target)
      throw(BaseException<enum RIFActType>, BadAllocException);
  RIFAction(const RIFTerm &object, const RIFTerm &attr)
      throw(BadAllocException);
  RIFAction(const RIFTerm &object) throw(BadAllocException);
  RIFAction(const RIFAction &copy) throw(BadAllocException);
  ~RIFAction() throw();

  static int cmp(const RIFAction &a1, const RIFAction &a2) throw();
  static bool cmplt0(const RIFAction &a1, const RIFAction &a2) throw();
  static bool cmpeq0(const RIFAction &a1, const RIFAction &a2) throw();
  static RIFAction parse(DPtr<uint8_t> *utf8str)
      throw(BadAllocException, SizeUnknownException,
            InvalidCodepointException, InvalidEncodingException,
            MalformedIRIRefException, TraceableException);
  
  DPtr<uint8_t> *toUTF8String() const throw(BadAllocException);
  bool equals(const RIFAction &rhs) const throw();
  RIFAction &normalize() throw(BadAllocException);
  enum RIFActType getType() const throw();
  bool isGround() const throw();
  void getVars(VarSet &vars) const throw();

  RIFAtomic getTargetAtomic() const
      throw(BaseException<enum RIFActType>);
  pair<RIFTerm, RIFTerm> getTargetTermPair() const
      throw(BaseException<enum RIFActType>);
  RIFTerm getTargetTerm() const
      throw(BaseException<enum RIFActType>);

  RIFAction &operator=(const RIFAction &rhs) throw(BadAllocException);
};

}

#include "rif/RIFAction-inl.h"

#endif /* __RIF__RIFACTION_H__ */
