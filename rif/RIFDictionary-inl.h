#include "rif/RIFDictionary.h"

namespace rif {

using namespace ptr;
using namespace std;

inline
RIFDictionary::RIFDictionary() throw()
    : term2id(Term2IDMap(RIFTerm::cmplt0)) {
  // do nothing
}

inline
RIFDictionary::RIFDictionary(const RIFDictionary &copy) throw()
    : term2id(copy.term2id), id2term(copy.id2term) {
  // do nothing
}

inline
RIFDictionary::~RIFDictionary() throw() {
  // do nothing
}

inline
enum RIFIDType RIFDictionary::getIDType(const uint64_t id) const throw() {
  return id >= RESERVED_IDS ? TERM :
        (id == UINT64_C(0) ? UNBOUND : ATOMIC_TYPE);
}

}
