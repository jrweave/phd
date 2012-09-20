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

using namespace std;

inline
RIFAction::RIFAction(const RIFTerm &object, const RIFTerm &attr)
    throw(BadAllocException)
    : type(RETRACT_SLOTS) {
  try {
    NEW(this->target, WHOLE(pair<RIFTerm, RIFTerm>), object, attr);
  } RETHROW_BAD_ALLOC
}

inline
RIFAction::RIFAction(const RIFTerm &object) throw(BadAllocException)
    : type(RETRACT_OBJECT) {
  try {
    NEW(this->target, RIFTerm, object);
  } RETHROW_BAD_ALLOC
}

inline
bool RIFAction::cmplt0(const RIFAction &a1, const RIFAction &a2) throw() {
  return RIFAction::cmp(a1, a2) < 0;
}

inline
bool RIFAction::cmpeq0(const RIFAction &a1, const RIFAction &a2) throw() {
  return RIFAction::cmp(a1, a2) == 0;
}

inline
bool RIFAction::equals(const RIFAction &rhs) const throw() {
  return RIFAction::cmp(*this, rhs) == 0;
}

inline
enum RIFActType RIFAction::getType() const throw() {
  return this->type;
}

}
