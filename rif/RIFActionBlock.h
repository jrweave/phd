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

#ifndef __RIF__RIFACTIONBLOCK_H__
#define __RIF__RIFACTIONBLOCK_H__

#include <map>
#include "rif/RIFAction.h"

namespace rif {

using namespace ptr;
using namespace std;

class RIFActVarBind {
private:
  RIFVar var;
  RIFAtomic *frame;
public:
  RIFActVarBind() throw(); // (?"" New())
  RIFActVarBind(const RIFVar &var) throw();
  RIFActVarBind(const RIFVar &var, const RIFAtomic &frame)
    throw(BadAllocException, TraceableException);
  RIFActVarBind(const RIFActVarBind &copy) throw(BadAllocException);
  ~RIFActVarBind() throw();

  static int cmp(const RIFActVarBind &b1, const RIFActVarBind &b2) throw();
  static bool cmplt0(const RIFActVarBind &b1, const RIFActVarBind &b2) throw();
  static bool cmpeq0(const RIFActVarBind &b1, const RIFActVarBind &b2) throw();
  static RIFActVarBind parse(DPtr<uint8_t> *utf8str)
      throw(BadAllocException, SizeUnknownException,
            InvalidCodepointException, InvalidEncodingException,
            MalformedIRIRefException, TraceableException);

  DPtr<uint8_t> *toUTF8String() const throw(BadAllocException);
  bool equals(const RIFActVarBind &rhs) const throw();
  RIFActVarBind &normalize() throw(BadAllocException);

  bool isGround() const throw(); // always false
  bool isGround(const bool include_act_var) const throw();
  void getVars(VarSet &vars) const throw(); // getVars(vars, true)
  void getVars(VarSet &vars, const bool include_act_var) const throw();

  bool isNew() const throw();
  RIFAtomic getFrame() const throw(TraceableException);
  RIFVar getActVar() const throw();

  RIFActVarBind &operator=(const RIFActVarBind &rhs) throw(BadAllocException);
};

typedef map<RIFVar,
            RIFActVarBind,
            bool(*)(const RIFVar &, const RIFVar &)>
        ActVarMap;

class RIFActionBlock {
private:
  DPtr<RIFAction> *actions; // most have size >= 1
  ActVarMap actvars;
public:
  RIFActionBlock() throw(BadAllocException);
  RIFActionBlock(DPtr<RIFAction> *acts)
    throw(BaseException<void*>, SizeUnknownException, TraceableException);
  RIFActionBlock(DPtr<RIFAction> *acts, const ActVarMap &actvars)
    throw(BaseException<void*>, SizeUnknownException, TraceableException);
  RIFActionBlock(const RIFActionBlock &copy) throw();
  ~RIFActionBlock() throw();

  static int cmp(const RIFActionBlock &a1, const RIFActionBlock &a2) throw();
  static bool cmplt0(const RIFActionBlock &a1, const RIFActionBlock &a2)
      throw();
  static bool cmpeq0(const RIFActionBlock &a1, const RIFActionBlock &a2)
      throw();
  static RIFActionBlock parse(DPtr<uint8_t> *utf8str)
      throw(BadAllocException, SizeUnknownException,
            InvalidCodepointException, InvalidEncodingException,
            MalformedIRIRefException, TraceableException);

  DPtr<uint8_t> *toUTF8String() const throw(BadAllocException);
  bool equals(const RIFActionBlock &rhs) const throw();
  RIFActionBlock &normalize() throw(BadAllocException);
  bool isGround() const throw(); // isGround(true)
  bool isGround(const bool include_act_var) const throw();
  void getVars(VarSet &vars) const throw(); // getVars(vars, true)
  void getVars(VarSet &vars, const bool include_act_var) const throw();

  bool getActVars(VarSet *vars) const throw();
  RIFActVarBind getBinding(const RIFVar &var) const throw(TraceableException);
  DPtr<RIFAction> *getActions() const throw();

  RIFActionBlock &operator=(const RIFActionBlock &rhs)
      throw(BadAllocException);
};

}

#include "rif/RIFActionBlock-inl.h"

#endif /* __RIF__RIFACTIONBLOCK_H__ */
