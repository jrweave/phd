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

#ifndef __RDF__RDFDICTENCREADER_H__
#define __RDF__RDFDICTENCREADER_H__

#include "io/InputStream.h"
#include "io/IOException.h"
#include "rdf/RDFDictionary.h"
#include "rdf/RDFReader.h"

namespace rdf {

using namespace io;

template<typename ID=RDFID<8>, typename ENC=RDFEncoder<ID> >
class RDFDictEncReader : public RDFReader {
private:
  InputStream *input;
  RDFDictionary<ID, ENC> *dict;
  DPtr<uint8_t> *buffer;
  size_t offset;
  bool discard_atypical;
  bool own_dictionary;
  bool readID(ID &id);
  static DPtr<uint8_t> *readNext(InputStream *is, const size_t len);
public:
  RDFDictEncReader(InputStream *is, RDFDictionary<ID, ENC> *dict,
                   const bool discard_atypical, const bool own_dictionary)
      throw(BaseException<void*>, IOException);
  virtual ~RDFDictEncReader() throw(IOException);

  static RDFDictionary<ID, ENC> *readDictionary(InputStream *is,
      RDFDictionary<ID, ENC> *dict);
  
  bool read(RDFTriple &triple);
  void close();
  RDFDictionary<ID, ENC> *getDictionary();
};

}

#include "rdf/RDFDictEncReader-inl.h"

#endif /* __RDF__RDFDICTENCREADER_H__ */
