#ifndef __RDF__NTRIPLESWRITER_H__
#define __RDF__NTRIPLESWRITER_H__

#include "ex/BaseException.h"
#include "io/OutputStream.h"
#include "ptr/BadAllocException.h"
#include "rdf/RDFWriter.h"
#include "sys/ints.h"
#include "ucs/InvalidEncodingException.h"

namespace rdf {

using namespace ex;
using namespace io;
using namespace ptr;
using namespace std;
using namespace ucs;

class NTriplesWriter : public RDFWriter {
private:
  OutputStream *output;
public:
  NTriplesWriter(OutputStream *os) throw(BaseException<void*>);
  ~NTriplesWriter() throw();

  static DPtr<uint8_t> *escapeUnicode(DPtr<uint8_t> *utf8str)
      throw(BadAllocException, InvalidEncodingException);
  static RDFTerm sanitize(const RDFTerm &bnode)
      throw(BadAllocException);

  void write(const RDFTriple &triple);
  void close();
};

}

#endif /* __RDF__NTRIPLESWRITER_H__ */
