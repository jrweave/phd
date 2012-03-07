#include "rdf/RDFTerm.h"

namespace rdf {

using namespace ex;
using namespace iri;
using namespace lang;
using namespace ptr;
using namespace std;

inline
RDFTerm::RDFTerm() throw()
    : type(BNODE), iri(NULL), lang(NULL), bytes(NULL), normalized(false) {
  // do nothing
}

inline
bool RDFTerm::cmplt0(const RDFTerm &term1, const RDFTerm &term2) throw() {
  return RDFTerm::cmp(term1, term2) < 0;
}

inline
bool RDFTerm::cmpeq0(const RDFTerm &term1, const RDFTerm &term2) throw() {
  return RDFTerm::cmp(term1, term2) == 0;
}

inline
enum RDFTermType RDFTerm::getType() const throw() {
  return this->type;
}

inline
bool RDFTerm::isAnonymous() const throw() {
  return this->type == BNODE && this->bytes == NULL;
}

inline
bool RDFTerm::isLiteral() const throw() {
  return this->isPlainLiteral() || this->type == TYPED_LITERAL;
}

inline
bool RDFTerm::isPlainLiteral() const throw() {
  return this->type == SIMPLE_LITERAL || this->type == LANG_LITERAL;
}

inline
bool RDFTerm::equals(const RDFTerm &term) const throw() {
  return RDFTerm::cmp(*this, term) == 0;
}

}
