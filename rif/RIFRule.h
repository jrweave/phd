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

#ifndef __RIF__RIFRULE_H__
#define __RIF__RIFRULE_H__

#include "rif/RIFActionBlock.h"
#include "rif/RIFCondition.h"

namespace rif {

using namespace ptr;
using namespace std;

class RIFRule {
private:
  RIFCondition condition;
  RIFActionBlock actionblock;
public:
  RIFRule() throw();
  RIFRule(const RIFCondition &cond) throw(BadAllocException);
  RIFRule(const RIFActionBlock &ab) throw(BadAllocException);
  RIFRule(const RIFCondition &cond, const RIFActionBlock &ab)
      throw(BadAllocException);
  ~RIFRule() throw();

  static int cmp(const RIFRule &r1, const RIFRule &r2) throw();
  static bool cmplt0(const RIFRule &r1, const RIFRule &r2) throw();
  static bool cmpeq0(const RIFRule &r1, const RIFRule &r2) throw();
  static RIFRule parse(DPtr<uint8_t> *utf8str)
      throw(BadAllocException, SizeUnknownException,
            InvalidCodepointException, InvalidEncodingException,
            MalformedIRIRefException, TraceableException);

  DPtr<uint8_t> *toUTF8String() const throw(BadAllocException);
  bool equals(const RIFRule &rhs) const throw();
  RIFRule &normalize() throw(BadAllocException);
  bool isGround() const throw(); // isGround(true)
  bool isGround(const bool include_act_var) const throw();
  void getVars(VarSet &vars) const throw(); // getVars(vars, true)
  void getVars(VarSet &vars, const bool include_act_var) const throw();

  RIFCondition getCondition() const throw();
  RIFActionBlock getActionBlock() const throw();

  RIFRule &operator=(const RIFRule &rhs) throw(BadAllocException);
};

}

#include "rif/RIFRule-inl.h"

#endif /* __RIF__RIFRULE_H__ */
