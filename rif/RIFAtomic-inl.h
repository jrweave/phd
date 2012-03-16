#include "rif/RIFAtomic.h"

namespace rif {

using namespace std;

inline
bool RIFAtomic::cmplt0(const RIFAtomic &a1, const RIFAtomic &a2) throw() {
  return RIFAtomic::cmp(a1, a2) < 0;
}

inline
bool RIFAtomic::cmpeq0(const RIFAtomic &a1, const RIFAtomic &a2) throw() {
  return RIFAtomic::cmp(a1, a2) == 0;
}

inline
bool RIFAtomic::equals(const RIFAtomic &rhs) const throw() {
  return RIFAtomic::cmp(*this, rhs) == 0;
}

inline
enum RIFAtomicType RIFAtomic::getType() const throw() {
  return this->type;
}


}
