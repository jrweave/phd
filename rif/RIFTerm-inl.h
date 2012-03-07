#include "rif/RIFTerm.h"

namespace rif {

inline
RIFTerm::RIFTerm() throw()
    : state(NULL), type(LIST) {
  // do nothing
}

inline
bool RIFTerm::cmplt0(const RIFTerm &t1, const RIFTerm &t2) throw() {
  return RIFTerm::cmp(t1, t2) < 0;
}

inline
bool RIFTerm::cmpeq0(const RIFTerm &t1, const RIFTerm &t2) throw() {
  return RIFTerm::cmp(t1, t2) == 0;
}

inline
bool RIFTerm::equals(const RIFTerm &rhs) const throw() {
  return RIFTerm::cmp(*this, rhs) == 0;
}

inline
enum RIFTermType RIFTerm::getType() const throw() {
  return this->type;
}

inline
bool RIFTerm::isSimple() const throw() {
  return this->type == VARIABLE || this->type == CONSTANT;
}

inline
bool RIFTerm::isWellFormed() const throw() {
  ContextMap contexts(RIFConst::cmplt0);
  return this->isWellFormed(contexts, NULL);
}

}
