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

inline
RIFRule::RIFRule() throw() {
  // do nothing
}

inline
RIFRule::RIFRule(const RIFCondition &cond) throw(BadAllocException) {
  try {
    this->condition = cond;
  } JUST_RETHROW(BadAllocException, "Cannot instantiate RIFRule.")
}

inline
RIFRule::RIFRule(const RIFActionBlock &ab) throw(BadAllocException) {
  try {
    this->actionblock = ab;
  } JUST_RETHROW(BadAllocException, "Cannot instantiate RIFRule.");
}

inline
RIFRule::RIFRule(const RIFCondition &cond, const RIFActionBlock &ab)
    throw(BadAllocException) {
  try {
    this->condition = cond;
    this->actionblock = ab;
  } JUST_RETHROW(BadAllocException, "Cannot instantiate RIFRule.");
}

inline
RIFRule::~RIFRule() throw() {
  // do nothing
}

inline
bool RIFRule::cmplt0(const RIFRule &r1, const RIFRule &r2) throw() {
  return RIFRule::cmp(r1, r2) < 0;
}

inline
bool RIFRule::cmpeq0(const RIFRule &r1, const RIFRule &r2) throw() {
  return RIFRule::cmp(r1, r2) == 0;
}

inline
bool RIFRule::equals(const RIFRule &rhs) const throw() {
  return RIFRule::cmp(*this, rhs) == 0;
}

inline
RIFRule &RIFRule::normalize() THROWS(BadAllocException) {
  this->condition.normalize();
  this->actionblock.normalize();
  return *this;
}
TRACE(BadAllocException, "Unable to normalize RIFRule.")

inline
bool RIFRule::isGround() const throw() {
  return this->isGround(true);
}

inline
bool RIFRule::isGround(const bool include_act_var) const throw() {
  return this->condition.isGround() &&
         this->actionblock.isGround(include_act_var);
}

inline
void RIFRule::getVars(VarSet &vars) const throw() {
  return this->getVars(vars, true);
}

inline
void RIFRule::getVars(VarSet &vars, const bool include_act_var) const throw() {
  this->condition.getVars(vars);
  this->actionblock.getVars(vars, include_act_var);
}

inline
RIFCondition RIFRule::getCondition() const throw() {
  return this->condition;
}

inline
RIFActionBlock RIFRule::getActionBlock() const throw() {
  return this->actionblock;
}

}
