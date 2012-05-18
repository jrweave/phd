#ifndef __RDF__NTRIPLESREADER_H__
#define __RDF__NTRIPLESREADER_H__

#include "io/InputStream.h"
#include "rdf/RDFReader.h"

namespace rdf {

using namespace io;

class NTriplesReader : public RDFReader {
private:
  InputStream *input;
  DPtr<uint8_t> *buffer;
  size_t offset;
public:
  NTriplesReader(InputStream *is) throw();
  ~NTriplesReader() throw();
  
  bool read(RDFTriple &triple);
  void close();
};

}

#endif /* __RDF__NTRIPLESREADER_H__ */
