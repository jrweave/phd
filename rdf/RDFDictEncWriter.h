/* Copyright 2012 Jesse Weaver
 * 
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 * 
 *        http://www.apache.org/licenses/LICENSE-2.0
 * 
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 *    implied. See the License for the specific language governing
 *    permissions and limitations under the License.
 */

#ifndef __RDF__RDFDICTENCWRITER_H__
#define __RDF__RDFDICTENCWRITER_H__

#include "ex/BaseException.h"
#include "io/IOException.h"
#include "ptr/BadAllocException.h"
#include "rdf/RDFDictionary.h"
#include "rdf/RDFTriple.h"
#include "rdf/RDFWriter.h"

namespace rdf {

using namespace ex;
using namespace io;
using namespace ptr;

template<typename ID=RDFID<8>, typename ENC=RDFEncoder<ID> >
class RDFDictEncWriter : public RDFWriter {
private:
  OutputStream *output;
  RDFDictionary<ID, ENC> *dict;
  DPtr<uint8_t> *buffer;
  bool own_dictionary;
public:
  RDFDictEncWriter(OutputStream *os, RDFDictionary<ID, ENC> *dict,
                   const bool own_dictionary)
      throw(BaseException<void*>, BadAllocException);
  virtual ~RDFDictEncWriter() throw(IOException);

  static void writeDictionary(OutputStream *os, RDFDictionary<ID, ENC> *dict);

  void write(const RDFTriple &triple);
  void close();
  RDFDictionary<ID, ENC> *getDictionary();
};

}

#include "rdf/RDFDictEncWriter-inl.h"

#endif /* __RDF__RDFDICTENCWRITER_H__ */
