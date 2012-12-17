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

#ifndef __PAR__DISTRDFDICTDECODE_H__
#define __PAR__DISTRDFDICTDECODE_H__

#include <list>
#include <vector>
#include "io/InputStream.h"
#include "par/DistComputation.h"
#include "par/DistRDFDictEncode.h"
#include "rdf/RDFDictionary.h"
#include "rdf/RDFWriter.h"
#include "rdf/RDFTriple.h"
#include "util/hash.h"

namespace par {

using namespace io;
using namespace rdf;
using namespace std;
using namespace util;

template<size_t N, typename ID=RDFID<N>, typename ENC=RDFEncoder<ID> >
class DistRDFDictDecode : public DistComputation {
private:
  typedef map<uint32_t, ID> I2TermMap;
  typedef map<ID, uint32_t> Term2IMap;
  struct pending_triple {
    RDFTerm parts[3];
    uint8_t need;
  };
  struct pending_position {
    typename list<pending_triple>::iterator triple;
    uint8_t pos;
  };
  struct pending_response {
    RDFTerm term;
    uint32_t n;
    int send_to;
  };
  list<pending_response> pending_responses;
  list<pending_triple> pending_triples;
  multimap<uint32_t, pending_position> pending_positions;
  Term2IMap pending_term2i;
  I2TermMap pending_i2term;
  DPtr<uint8_t> *current;
  typename list<pending_triple>::iterator curpend;
  RDFWriter *writer;
  InputStream *input;
  RDFDictionary<ID, ENC> *dict;
  int ndonesent;
  int nprocdone;
  int nproc;
  int rank;
  uint32_t count;
  uint8_t curpos;
  bool gotten;
  bool discard_atypical;
protected:
  virtual void start() throw(TraceableException);
  virtual int pickup(DPtr<uint8_t> *&buffer, size_t &len)
      throw(BadAllocException, TraceableException);
  virtual void dropoff(DPtr<uint8_t> *msg) throw(TraceableException);
  virtual void finish() throw(TraceableException);
  virtual void fail() throw();
public:
  DistRDFDictDecode(const int rank, const int nproc, RDFWriter *writer,
      Distributor *dist, InputStream *in, RDFDictionary<ID, ENC> *dict,
      const bool discard_atypical)
      throw(BaseException<void*>, BadAllocException);
  DistRDFDictDecode(const int rank, const int nproc, RDFWriter *writer,
      Distributor *dist, InputStream *in, ENC &enc,
      RDFDictionary<ID, ENC> *dict, const bool discard_atypical)
      throw(BaseException<void*>, BadAllocException);
  virtual ~DistRDFDictDecode() throw(DistException);
  DistRDFDictionary<N, ID, ENC> *getDictionary() throw();
};

}

#include "par/DistRDFDictDecode-inl.h"

#endif /* __PAR__DISTRDFDICTDECODE_H_ */
