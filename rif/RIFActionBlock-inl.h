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

inline
RIFActVarBind::RIFActVarBind() throw()
    : frame(NULL) {
  // do nothing
}

inline
RIFActVarBind::RIFActVarBind(const RIFVar &var) throw()
    : var(var), frame(NULL) {
  // do nothing
}

inline
bool RIFActVarBind::cmplt0(const RIFActVarBind &b1, const RIFActVarBind &b2)
    throw() {
  return RIFActVarBind::cmp(b1, b2) < 0;
}

inline
bool RIFActVarBind::cmpeq0(const RIFActVarBind &b1, const RIFActVarBind &b2)
    throw() {
  return RIFActVarBind::cmp(b1, b2) == 0;
}

inline
bool RIFActVarBind::equals(const RIFActVarBind &rhs) const throw() {
  return RIFActVarBind::cmp(*this, rhs) == 0;
}

inline
bool RIFActVarBind::isGround() const throw() {
  return false; // becaues of action variable
}

inline
bool RIFActVarBind::isGround(const bool include_act_var) const throw() {
  return include_act_var ? false
                         : (this->frame == NULL || this->frame->isGround());
}

inline
void RIFActVarBind::getVars(VarSet &vars) const throw() {
  return this->getVars(vars, true);
}

inline
bool RIFActVarBind::isNew() const throw() {
  return this->frame == NULL;
}

inline
RIFAtomic RIFActVarBind::getFrame() const throw(TraceableException) {
  if (this->frame == NULL) {
    THROW(TraceableException, "Action variable is bound to New().");
  }
  return *(this->frame);
}

inline
RIFVar RIFActVarBind::getActVar() const throw() {
  return this->var;
}

inline
bool RIFActionBlock::cmplt0(const RIFActionBlock &a1, const RIFActionBlock &a2)
    throw() {
  return RIFActionBlock::cmp(a1, a2) < 0;
}

inline
bool RIFActionBlock::cmpeq0(const RIFActionBlock &a1, const RIFActionBlock &a2)
    throw() {
  return RIFActionBlock::cmp(a1, a2) == 0;
}

inline
bool RIFActionBlock::equals(const RIFActionBlock &rhs) const throw() {
  return RIFActionBlock::cmp(*this, rhs) == 0;
}

inline
bool RIFActionBlock::isGround() const throw() {
  return this->isGround(true);
}

inline
void RIFActionBlock::getVars(VarSet &vars) const throw() {
  return this->getVars(vars, true);
}

inline
DPtr<RIFAction> *RIFActionBlock::getActions() const throw() {
  this->actions->hold();
  return this->actions;
}

}
