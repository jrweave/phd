#ifndef __RDF__RDFWRITER_H__
#define __RDF__RDFWRITER_H__

#include "rdf/RDFTriple.h"

namespace rdf {

class RDFWriter {
public:
  virtual void write(const RDFTriple &triple) = 0;
  virtual void close() = 0;
};

}

#endif /* __RDF__RDFWRITER_H__ */
