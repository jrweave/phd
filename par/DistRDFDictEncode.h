#ifndef __PAR__DISTRDFDICTENCODE_H__
#define __PAR__DISTRDFDICTENCODE_H__

#include <list>
#include <vector>
#include "par/DistComputation.h"
#include "rdf/RDFDictionary.h"

namespace par {

using namespace rdf;
using namespace std;

class DistRDFDictEncode;

template<size_t N, typename ID=RDFID<N>, typename ENC=RDFEncoder<ID> >
class DistRDFDictionary : public RDFDictionary<ID, ENC> {
protected;
  int rank;
  virtual bool nextID(ID &id);
  virtual void set(const RDFTerm &term, const ID &id);
public:
  DistRDFDictionary(const int rank)
      throw(TraceableException);
  DistRDFDictionary(const int rank, const ENC &enc)
      throw(TraceableException);
  virtual ~DistRDFDictionary() throw();

  virtual ID encode(const RDFTerm &term);
  virtual ID locallyEncode(const RDFTerm &term);

  friend class DistRDFDictEncode;
};

template<size_t N, typename ID=RDFID<N>, typename ENC=RDFEncoder<ID> >
class DistRDFDictEncode : public DistComputation {
private:
  typedef map<uint32_t, RDFTerm> I2TermMap;
  typedef map<RDFTerm, uint32_t,
              bool(*)(const RDFTerm&, const RDFTerm&)>
          Term2IMap;
  struct pending_triple {
    ID parts[3];
    uint8_t need;
  };
  struct pending_position {
    list<pending_triple>::iterator triple;
    uint8_t pos;
  };
  struct pending_response {
    ID id;
    uint32_t n;
    int send_to;
  };
  list<pending_triple> pending_responses;
  list<pending_triple> pending_triples;
  multimap<uint32_t, pending_position> pending_positions;
  Term2IMap pending_term2i;
  I2TermMap pending_i2term;
  RDFTriple current;
  list<pending_triple>::iterator curpend;
  RDFReader *reader;
  OutputStream *output;
  DPtr<uint8_t> *outbuf;
  DistRDFDictionary<ID, ENC> *dict;
  int ndonesent;
  int nprocdone;
  int nproc;
  uint32_t count;
  uint8_t curpos;
  bool gotten;
protected:
  virtual void start() throw(TraceableException);
  virtual int pickup(DPtr<uint8_t> *&buffer, size_t &len)
      throw(BadAllocException, TraceableException);
  virtual void dropoff(DPtr<uint8_t> *msg) throw(TraceableException);
  virtual void finish() throw(TraceableException);
  virtual void fail() throw();
public:
  DistRDFDictEncode(const int rank, const int nproc, RDFReader *reader,
      Distributor *dist, OutputStream *out)
      throw(BaseException<void*>, BadAllocException);
  DistRDFDictEncode(const int rank, const int nproc, RDFReader *reader,
      Distributor *dist, OutputStream *out, ENC &enc)
      throw(BaseException<void*>, BadAllocException);
  virtual ~DistRDFDictEncode() throw(DistException);
  DistRDFDictionary<ID, ENC> *getDictionary() throw();
};

}

#include "par/DistRDFDictEncode-inl.h"

#endif /* __PAR__DISTRDFDICTENCODE_H_ */