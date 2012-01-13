#ifndef __RDF__RDFTERM_H__
#define __RDF__RDFTERM_H__

#include <iostream>
#include "ex/BaseException.h"
#include "iri/IRIRef.h"
#include "lang/LangTag.h"
#include "ptr/DPtr.h"
#include "ptr/SizeUnknownException.h"
#include "sys/ints.h"

namespace rdf {

using namespace ex;
using namespace iri;
using namespace lang;
using namespace ptr;
using namespace std;

enum RDFTermType {
  BNODE = 1,
  IRI = 2,
  SIMPLE_LITERAL = 3,
  LANG_LITERAL = 4,
  TYPED_LITERAL = 5
};

// Considered here solely as a SYNTACTIC entity;
// i.e., no interpretation, scoping, etc.
class RDFTerm {
private:
  IRIRef *iri;
  LangTag *lang;
  DPtr<uint8_t> *bytes;
  enum RDFTermType type;
public:
  RDFTerm() throw(); // anonymous blank node
  RDFTerm(DPtr<uint8_t> *label) throw(SizeUnknownException);
  RDFTerm(const IRIRef &iriref)
      throw(BaseException<IRIRef>, BadAllocException);
  RDFTerm(DPtr<uint8_t> *utf8lex, const LangTag *lang)
      throw(SizeUnknownException, BaseException<void*>, BadAllocException);
  RDFTerm(DPtr<uint8_t> *utf8lex, const IRIRef &datatype)
      throw(SizeUnknownException, BaseException<void*>, BadAllocException);
  RDFTerm(const RDFTerm &copy) throw(BadAllocException);
  ~RDFTerm() throw();

  // Static Methods
  static int cmp(const RDFTerm &term1, const RDFTerm &term2) throw();
  static bool cmplt0(const RDFTerm &term1, const RDFTerm &term2) throw();
  static bool cmpeq0(const RDFTerm &term1, const RDFTerm &term2) throw();
  static DPtr<uint8_t> *escape(DPtr<uint8_t> *str, bool as_iri)
      throw(SizeUnknownException, BadAllocException, BaseException<void*>);
  static DPtr<uint8_t> *unescape(DPtr<uint8_t> *str, bool as_iri)
      throw(SizeUnknownException, BadAllocException, BaseException<void*>,
            TraceableException);
  static RDFTerm parse(DPtr<uint8_t> *utf8str)
      throw(SizeUnknownException, BaseException<void*>, TraceableException);

  // Final Methods
  enum RDFTermType getType() const throw();
  bool isAnonymous() const throw();
  bool isLiteral() const throw();
  bool isPlainLiteral() const throw();
  DPtr<uint8_t> *toUTF8String() const throw(BadAllocException);
  bool equals(const RDFTerm &term) const throw();

  // BNODE
  DPtr<uint8_t> *getLabel() throw(BaseException<enum RDFTermType>);

  // IRI
  IRIRef getIRIRef() const throw(BaseException<enum RDFTermType>);

  // SIMPLE_LITERAL, LANG_LITERAL, and TYPED_LITERAL
  DPtr<uint8_t> *getLexForm() throw(BaseException<enum RDFTermType>);

  // LANG_LITERAL
  LangTag getLangTag() const throw(BaseException<enum RDFTermType>);

  // TYPED_LITERAL
  IRIRef getDatatype() const throw(BaseException<enum RDFTermType>);

  RDFTerm &operator=(const RDFTerm &rhs) throw(BadAllocException);
};

} // end namespace rdf

// TODO
//std::istream& operator>>(std::istream &stream, rdf::RDFTerm &term);
//std::ostream& operator<<(std::ostream &stream, const rdf::RDFTerm &term);

#include "rdf/RDFTerm-inl.h"

#endif /* __RDF__RDFTERM_H__ */
