#include "rif/RIFCondition.h"

namespace rif {

using namespace ptr;
using namespace std;

inline
RIFCondition::RIFCondition() throw()
    : type(DISJUNCTION), state(NULL) {
  // do nothing; Or() is false
}

inline
bool RIFCondition::cmplt0(const RIFCondition &c1, const RIFCondition &c2)
    throw() {
  return RIFCondition::cmp(c1, c2) < 0;
}

inline
bool RIFCondition::cmpeq0(const RIFCondition &c1, const RIFCondition &c2)
    throw() {
  return RIFCondition::cmp(c1, c2) == 0;
}

inline
bool RIFCondition::equals(const RIFCondition &rhs) const throw() {
  return RIFCondition::cmp(*this, rhs) == 0;
}

inline
enum RIFCondType RIFCondition::getType() const throw() {
  return this->type;
}

inline
bool RIFCondition::isGround() const throw() {
  VarSet vars(RIFVar::cmplt0);
  this->getVars(vars);
  return vars.size() == 0;
}

}
