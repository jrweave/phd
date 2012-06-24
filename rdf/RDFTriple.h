#ifndef __RDF__RDFTRIPLE_H__
#define __RDF__RDFTRIPLE_H__

#include "ex/BaseException.h"
#include "iri/IRIRef.h"
#include "lang/LangTag.h"
#include "ptr/DPtr.h"
#include "rdf/RDFTerm.h"
#include "sys/ints.h"
#include "ucs/InvalidCodepointException.h"
#include "ucs/InvalidEncodingException.h"

namespace rdf {

using namespace ex;
using namespace iri;
using namespace lang;
using namespace ptr;
using namespace std;
using namespace ucs;

class RDFTriple {
private:
  RDFTerm subject, predicate, object;
public:
  RDFTriple() throw();
  RDFTriple(const RDFTerm &subj, const RDFTerm &pred, const RDFTerm &obj)
      throw(BaseException<enum RDFTermType>, BadAllocException);
  RDFTriple(const RDFTriple &copy) throw(BadAllocException);
  virtual ~RDFTriple() throw();

  static int cmp(const RDFTriple &t1, const RDFTriple &t2) throw();
  static bool cmplt0(const RDFTriple &t1, const RDFTriple &t2) throw();
  static bool cmpeq0(const RDFTriple &t1, const RDFTriple &t2) throw();
  static RDFTriple parse(DPtr<uint8_t> *utf8str)
      throw(SizeUnknownException, BadAllocException, TraceableException,
            InvalidEncodingException, InvalidCodepointException,
            MalformedIRIRefException, MalformedLangTagException);

  DPtr<uint8_t> *toUTF8String() const throw(BadAllocException);
  DPtr<uint8_t> *toUTF8String(const bool with_dot_endl) const
      throw(BadAllocException);
  bool equals(const RDFTriple &t) const throw();
  RDFTriple &normalize() throw(BadAllocException, TraceableException);

  enum RDFTermType getSubjType() const throw();
  RDFTerm getSubj() const throw();
  IRIRef getSubjIRI() const throw(BaseException<enum RDFTermType>);

  RDFTerm getPred() const throw();
  IRIRef getPredIRI() const throw();

  enum RDFTermType getObjType() const throw();
  RDFTerm getObj() const throw();
  IRIRef getObjIRI() const throw(BaseException<enum RDFTermType>);

  RDFTriple &operator=(const RDFTriple &rhs) throw(BadAllocException);
};

}

// TODO
//std::istream& operator>>(std::istream &stream, rdf::RDFTriple &triple);
std::ostream& operator<<(std::ostream &stream, const rdf::RDFTriple &triple);

#include "rdf/RDFTriple-inl.h"

#endif /* __RDF__RDFTRIPLE_H__ */
