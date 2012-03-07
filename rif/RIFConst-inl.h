#include "rif/RIFConst.h"

namespace rif {

using namespace iri;
using namespace ptr;
using namespace std;

inline
RIFConst::RIFConst(const RIFConst &copy) throw()
    : lex(copy.lex), datatype(copy.datatype), normalized(copy.normalized) {
  this->lex->hold();
}

inline
RIFConst::~RIFConst() throw() {
  this->lex->drop();
}

inline
bool RIFConst::cmplt0(const RIFConst &rc1, const RIFConst &rc2) throw() {
  return RIFConst::cmp(rc1, rc2) < 0;
}

inline
bool RIFConst::cmpeq0(const RIFConst &rc1, const RIFConst &rc2) throw() {
  return RIFConst::cmp(rc1, rc2) == 0;
}

inline
bool RIFConst::equals(const RIFConst &rhs) const throw() {
  return RIFConst::cmp(*this, rhs) == 0;
}

inline
DPtr<uint8_t> *RIFConst::getLexForm() const throw() {
  this->lex->hold();
  return this->lex;
}

inline
IRIRef RIFConst::getDatatype() const throw() {
  return this->datatype;
}

}
