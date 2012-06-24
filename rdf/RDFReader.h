#ifndef __RDF__RDFREADER_H__
#define __RDF__RDFREADER_H__

#include "rdf/RDFTriple.h"

namespace rdf {

class RDFReader {
public:
  virtual ~RDFReader() {}
  virtual bool read(RDFTriple &triple) = 0;
  virtual void close() = 0;
};

}

#endif /* __RDF__RDFREADER_H__ */
