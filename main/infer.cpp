#include <cstdlib>
#include <deque>
#include <map>
#include <set>
#include "io/InputStream.h"
#include "io/OutputStream.h"
#include "par/DistComputation.h"
#include "par/DistRDFDictEncode.h"
#include "par/Distributor.h"
#include "par/MPIDelimFileInputStream.h"
#include "par/MPIDistPtrFileOutputStream.h"
#include "par/MPIPacketDistributor.h"
#include "par/StringDistributor.h"
#include "ptr/alloc.h"
#include "ptr/DPtr.h"
#include "ptr/MPtr.h"
#include "rdf/NTriplesReader.h"
#include "rdf/NTriplesWriter.h"
#include "rdf/RDFReader.h"
#include "rdf/RDFDictionary.h"
#include "util/timing.h"

#define COORDEVERY 100000
#define NUMREQUESTS 4
#define PACKETBYTES 128
#define PAGEBYTES (1 << 20)
#define IDBYTES 8
#define RANDOMIZE true

#if TIMING_USE != TIMING_NONE
#define MARK_TIME(t) \
  MPI::COMM_WORLD.Barrier(); \
  if (rank == 0) { \
    t = TIME(); \
  } \
  MPI::COMM_WORLD.Barrier()
#else
#define MARK_TIME(t)
#endif

#define USING_SET 1

#if USING_SET
#define STORAGE set
#define INSERT insert
#define LOWER_BOUND(store, pattern) (store.lower_bound(pattern))
#define UPPER_BOUND(store, pattern) (store.upper_bound(pattern))
#endif

#define RDF_TERM(name, cstr) \
  DPtr<uint8_t> * _ ## name ## str; \
  NEW(_ ## name ## str, MPtr<uint8_t>, strlen(cstr)); \
  ascii_strcpy(_ ## name ## str->dptr(), cstr); \
  RDFTerm name = RDFTerm::parse(_ ## name ## str); \
  _ ## name ## str->drop()


using namespace io;
using namespace par;
using namespace rdf;
using namespace std;

typedef RDFID<IDBYTES> ID;
typedef DistRDFDictionary<IDBYTES> Dict;
typedef struct {
  ID parts[3];
} IDTrip;

Dict *GLOBAL_DICT;

bool pso_order(const IDTrip &i1, const IDTrip &i2) {
  return i1.parts[1] < i2.parts[1] ||
         (i1.parts[1] == i2.parts[1] && i1.parts[0] <  i2.parts[0]) ||
         (i1.parts[1] == i2.parts[1] && i1.parts[0] == i2.parts[0] && i1.parts[2] < i2.parts[2]);
}

inline
bool operator<(const IDTrip &i1, const IDTrip &i2) {
  return pso_order(i1, i2);
}

class StorageLoader : public OutputStream {
private:
  STORAGE<IDTrip> *trips;
public:
  StorageLoader(STORAGE<IDTrip> &s) : trips(&s) {}
  ~StorageLoader() throw(IOException) {}
  void close() throw(IOException) {}
  void flush() throw(IOException) {}
  void write(DPtr<uint8_t> *buf, size_t &nwritten)
      throw(IOException, SizeUnknownException, BaseException<void*>) {
    IDTrip trip;
    const uint8_t *mark = buf->dptr();
    memcpy(&(trip.parts[0]), mark, IDBYTES);
    mark += IDBYTES;
    memcpy(&(trip.parts[1]), mark, IDBYTES);
    mark += IDBYTES;
    memcpy(&(trip.parts[2]), mark, IDBYTES);
    trips->INSERT(trip);
    nwritten = buf->size();
  }
};

class StorageStream : public InputStream {
private:
  STORAGE<IDTrip> *trips;
  STORAGE<IDTrip>::const_iterator it;
  DPtr<uint8_t> *buf;
public:
  StorageStream(STORAGE<IDTrip> &s) : trips(&s), it(s.begin()) 
  { NEW(buf, MPtr<uint8_t>, 3*ID::size()); }
  ~StorageStream() throw(IOException) 
  { this->buf->drop(); }
  void close() throw(IOException) {}
  DPtr<uint8_t> *read(const int64_t amt)
      THROWS(IOException, BadAllocException) {
    if (this->it == this->trips->end()) {
      return NULL;
    }
    if (!buf->alone()) {
      buf = buf->stand();
    }
    IDTrip trip = *(this->it);
    ++this->it;
    uint8_t *w = buf->dptr();
    memcpy(w, &trip.parts[0], ID::size());
    w += ID::size();
    memcpy(w, &trip.parts[1], ID::size());
    w += ID::size();
    memcpy(w, &trip.parts[2], ID::size());
    buf->hold();
    return buf;
  }
  TRACE(IOException, "(trace)")
};

template<typename iter>
class MinRDFSDist : public DistComputation {
private:
  Dict *dict;
  STORAGE<IDTrip> *store;
  set<ID> *unknowns;
  set<ID> preds;
  iter mark, end;
  int rank, nproc;
  IDTrip last;
public:
  MinRDFSDist(int nproc, iter mark, iter end, Dict *dict, set<ID> *unknowns,
      STORAGE<IDTrip> &store, Distributor *dist) throw(BaseException<void*>)
      : DistComputation(dist), store(&store), nproc(nproc), rank(nproc),
        dict(dict), mark(mark), end(end), unknowns(unknowns) {
    RDF_TERM(subprop, "<http://www.w3.org/2000/01/rdf-schema#subPropertyOf>");
    RDF_TERM(subclass, "<http://www.w3.org/2000/01/rdf-schema#subClassOf>");
    RDF_TERM(domain, "<http://www.w3.org/2000/01/rdf-schema#domain>");
    RDF_TERM(range, "<http://www.w3.org/2000/01/rdf-schema#range>");
    ID id;
    if (dict->lookup(subprop, id)) {
      this->preds.insert(id);
    }
    if (dict->lookup(subclass, id)) {
      this->preds.insert(id);
    }
    if (dict->lookup(domain, id)) {
      this->preds.insert(id);
    }
    if (dict->lookup(range, id)) {
      this->preds.insert(id);
    }
  }
  virtual ~MinRDFSDist() throw (DistException) {}
  void start() throw(TraceableException) {}
  void finish() throw(TraceableException) {}
  void fail() throw() {}
  int pickup(DPtr<uint8_t> *&buffer, size_t &len)
      throw(BadAllocException, TraceableException) {
    if (this->rank >= this->nproc && this->mark == this->end) {
      return -2;
    }
    len = 3*ID::size();
    if (buffer->size() < len) {
      buffer->drop();
      try {
        NEW(buffer, MPtr<uint8_t>, len);
      } RETHROW_BAD_ALLOC
    }
    if (this->rank >= this->nproc) {
      this->last = *this->mark;
      ++this->mark;
    }
    uint8_t *write_to = buffer->dptr();
    memcpy(write_to, this->last.parts[0].ptr(), ID::size());
    write_to += ID::size();
    memcpy(write_to, this->last.parts[1].ptr(), ID::size());
    write_to += ID::size();
    memcpy(write_to, this->last.parts[2].ptr(), ID::size());
    int send_to;
    if (this->rank >= this->nproc) {
      if (this->preds.count(this->last.parts[1]) > 0) {
        this->rank = 1;
        send_to = 0;
      } else {
        send_to = rand() % this->nproc;
      }
    } else {
      send_to = this->rank;
      ++this->rank;
    }
    return send_to;
  }
  void dropoff(DPtr<uint8_t> *msg) throw(TraceableException) {
    IDTrip trip;
    const uint8_t *read_from = msg->dptr();
    memcpy(trip.parts[0].ptr(), read_from, ID::size());
    read_from += ID::size();
    memcpy(trip.parts[1].ptr(), read_from, ID::size());
    read_from += ID::size();
    memcpy(trip.parts[2].ptr(), read_from, ID::size());
    this->store->INSERT(trip);
    if (this->unknowns != NULL) {
      int i;
      for (i = 0; i < 3; ++i) {
        if (!this->dict->lookup(trip.parts[i])) {
          this->unknowns->insert(trip.parts[i]);
        }
      }
    }
  }
};

template<typename iter>
class DistLookupReq : public DistComputation {
private:
  iter mark, end;
  Dict *dict;
  set<ID> pending;
  multimap<ID, int> *requests;
public:
  DistLookupReq(iter begin, iter end, Dict *dict, multimap<ID, int> &reqs, Distributor *dist)
      throw(BaseException<void*>)
      : DistComputation(dist), mark(begin), end(end), dict(dict), requests(&reqs) {}
  virtual ~DistLookupReq() throw(DistException) {}
  void start() throw(TraceableException) {}
  void finish() throw(TraceableException) {}
  void fail() throw() {}
  int pickup(DPtr<uint8_t> *&buffer, size_t &len)
      throw(BadAllocException, TraceableException) {
    if (this->mark == this->end) {
      return -2;
    }
    if (this->pending.count(*this->mark) > 0) {
      ++this->mark;
      return -1;
    }
    len = ID::size() + sizeof(int);
    if (buffer->size() < len) {
      buffer->drop();
      try {
        NEW(buffer, MPtr<uint8_t>, len);
      } RETHROW_BAD_ALLOC
    }
    int send_to;
    int rank = MPI::COMM_WORLD.Get_rank();
    memcpy(&send_to, this->mark->ptr(), sizeof(int));
    memcpy(buffer->dptr(), this->mark->ptr(), ID::size());
    memcpy(buffer->dptr() + ID::size(), &rank, sizeof(int));
    ++this->mark;
    return send_to;
  }
  void dropoff(DPtr<uint8_t> *msg) throw(TraceableException) {
    ID id;
    int rank;
    memcpy(id.ptr(), msg->dptr(), ID::size());
    memcpy(&rank, msg->dptr() + ID::size(), sizeof(int));
    this->requests->insert(pair<ID, int>(id, rank));
  }
};

template<typename iter>
class DistLookupResp : public DistComputation {
private:
  iter begin, end;
  Dict *dict;
public:
  DistLookupResp(iter begin, iter end, Dict *dict, Distributor *dist)
      throw(BaseException<void*>)
      : DistComputation(dist), begin(begin), end(end), dict(dict) {}
  virtual ~DistLookupResp() throw(DistException) {}
  void start() throw(TraceableException) {}
  void finish() throw(TraceableException) {}
  void fail() throw() {}
  int pickup(DPtr<uint8_t> *&buffer, size_t &len)
      throw(BadAllocException, TraceableException) {
    if (this->begin == this->end) {
      return -2;
    }
    RDFTerm term = this->dict->decode(this->begin->first);
    DPtr<uint8_t> *str = term.toUTF8String();
    len = ID::size() + str->size();
    if (buffer->size() < len) {
      buffer->drop();
      try {
        NEW(buffer, MPtr<uint8_t>, len);
      } RETHROW_BAD_ALLOC
    }
    uint8_t *write_to = buffer->dptr();
    memcpy(write_to, this->begin->first.ptr(), ID::size());
    write_to += ID::size();
    memcpy(write_to, str->dptr(), str->size());
    str->drop();
    int send_to = this->begin->second;
    ++this->begin;
    return send_to;
  }
  void dropoff(DPtr<uint8_t> *msg) throw(TraceableException) {
    ID id;
    memcpy(id.ptr(), msg->dptr(), ID::size());
    DPtr<uint8_t> *str = msg->sub(ID::size(), msg->size() - ID::size());
    RDFTerm term = RDFTerm::parse(str);
    str->drop();
    this->dict->set(term, id);
  }
};

Dict *load(const char *filename, STORAGE<IDTrip> &store) {
  InputStream *is;
  int rank = MPI::COMM_WORLD.Get_rank();
  int size = MPI::COMM_WORLD.Get_size();
  NEW(is, MPIDelimFileInputStream, MPI::COMM_WORLD, filename,
      MPI::MODE_RDONLY, MPI::INFO_NULL, PAGEBYTES, to_ascii('\n'));
  RDFReader *rr;
  NEW(rr, NTriplesReader, is);
  Distributor *dist;
  NEW(dist, MPIPacketDistributor, MPI::COMM_WORLD, PACKETBYTES,
      NUMREQUESTS, COORDEVERY, 7);
  NEW(dist, StringDistributor, rank, PACKETBYTES, dist);
  DistRDFDictEncode<IDBYTES> *distcomp;
  OutputStream *out;
  NEW(out, StorageLoader, store);
  NEW(distcomp, DistRDFDictEncode<IDBYTES>, rank, size, rr, dist, out);
  distcomp->exec();
  Dict *dict = distcomp->getDictionary();
  DELETE(distcomp);
  return dict;
}

pair<IDTrip, IDTrip> store_range(const ID *parts[3]) {
  pair<IDTrip, IDTrip> rng;
  int i;
  for (i = 0; i < 3; ++i) {
    if (parts[i] == NULL) {
      rng.first.parts[i] = ID::min();
      rng.second.parts[i] = ID::max();
    } else {
      rng.first.parts[i] = rng.second.parts[i] = *parts[i];
    }
  }
  return rng;
}

pair<IDTrip, IDTrip> store_range(const ID *s, const ID *p, const ID *o) {
  const ID *parts[3];
  parts[0] = s;
  parts[1] = p;
  parts[2] = o;
  return store_range(parts);
}

void rdfs_replicate(Dict *dict, STORAGE<IDTrip> &facts) {
  int rank = MPI::COMM_WORLD.Get_rank();
  int nproc = MPI::COMM_WORLD.Get_size();
  set<ID> idset;
  multimap<ID, int> reqs;
  STORAGE<IDTrip> redist;
  Distributor *dist;
  DistComputation *distcomp;

  NEW(dist, MPIPacketDistributor, MPI::COMM_WORLD, 3*ID::size(),
      NUMREQUESTS, COORDEVERY, 8);
  NEW(distcomp, MinRDFSDist<STORAGE<IDTrip>::const_iterator>, nproc,
      facts.begin(), facts.end(), dict, &idset, redist, dist);
  distcomp->exec();
  DELETE(distcomp);
  cerr << "BEFORE SWAP: " << facts.size() << endl;
  facts.swap(redist);
  cerr << "AFTER SWAP: " << facts.size() << endl;

  NEW(dist, MPIPacketDistributor, MPI::COMM_WORLD, ID::size() + sizeof(int),
      NUMREQUESTS, COORDEVERY, 9);
  NEW(distcomp, DistLookupReq<set<ID>::iterator>, idset.begin(),
      idset.end(), dict, reqs, dist);
  distcomp->exec();
  DELETE(distcomp);

  NEW(dist, MPIPacketDistributor, MPI::COMM_WORLD, PACKETBYTES,
      NUMREQUESTS, COORDEVERY, 10);
  NEW(dist, StringDistributor, rank, PACKETBYTES, dist);
  NEW(distcomp, WHOLE(DistLookupResp<multimap<ID, int>::iterator>),
      reqs.begin(), reqs.end(), dict, dist);
  distcomp->exec();
  DELETE(distcomp);
}

#define OUT(idtrip) GLOBAL_DICT->decode((idtrip).parts[0]) << " " << GLOBAL_DICT->decode((idtrip).parts[1]) << " " << GLOBAL_DICT->decode((idtrip).parts[2]) << " ."


size_t rule_infer(STORAGE<IDTrip> &facts, ID *r[9], const int j1, const int j2, const int ipos[3]) {
  int rank = MPI::COMM_WORLD.Get_rank();
  TIME_T(begin_time, lhsidx_end_time, rhsidx_end_time, end_time);

  MARK_TIME(begin_time);

  size_t oldsize = facts.size();
  unsigned long nprobes = 0;
  unsigned long nmatches = 0;
  unsigned long nfirings = 0;
  unsigned long lhssize = 0;
  unsigned long rhssize = 0;
  multimap<ID, IDTrip> lhs, rhs;
  STORAGE<IDTrip>::const_iterator begin, end;
  pair<IDTrip, IDTrip> rng = store_range(r[0], r[1], r[2]);
  begin = LOWER_BOUND(facts, rng.first);
  end = UPPER_BOUND(facts, rng.second);
  for (; begin != end; ++begin) {
    lhs.insert(pair<ID, IDTrip>(begin->parts[j1], *begin));
    //cerr << "LHS: " << GLOBAL_DICT->decode(begin->parts[j1]) << " => " << OUT(*begin) << endl;
  }
  lhssize = (unsigned long) lhs.size();
  MARK_TIME(lhsidx_end_time);
  rng = store_range(r[3], r[4], r[5]);
  begin = LOWER_BOUND(facts, rng.first);
  end = UPPER_BOUND(facts, rng.second);
  for (; begin != end; ++begin) {
    rhs.insert(pair<ID, IDTrip>(begin->parts[j2-3], *begin));
    //cerr << "RHS: " << GLOBAL_DICT->decode(begin->parts[j2-3]) << " => " << OUT(*begin) << endl;
  }
  rhssize = (unsigned long) rhs.size();
  MARK_TIME(rhsidx_end_time);
  multimap<ID, IDTrip>::const_iterator lit = lhs.begin();
  deque<IDTrip> buffered;
  while (lit != lhs.end()) {
    //cerr << "JOIN " << GLOBAL_DICT->decode(lit->first) << endl;
    ++nprobes;
    pair<multimap<ID, IDTrip>::const_iterator,
         multimap<ID, IDTrip>::const_iterator> rhs_range
         = rhs.equal_range(lit->first);
    if (rhs_range.first == rhs.end()) {
      lit = lhs.end();
    } else {
      pair<multimap<ID, IDTrip>::const_iterator,
           multimap<ID, IDTrip>::const_iterator> lhs_range;
      lhs_range.first = lit;
      lhs_range.second = lhs.upper_bound(lit->first);
      for (; rhs_range.first != rhs_range.second; ++rhs_range.first) {
        //cerr << "PROBE " << OUT(rhs_range.first->second) << endl;
        for (lit = lhs_range.first; lit != lhs_range.second; ++lit) {
          ++nmatches;
          //cerr << "MATCH " << OUT(lit->second) << endl;
          IDTrip infer;
          int i;
          for (i = 0; i < 3; ++i) {
            infer.parts[i] = ipos[i] < 3 ? lit->second.parts[ipos[i]] :
                            (ipos[i] < 6 ?
                                rhs_range.first->second.parts[ipos[i]-3] :
                                *r[ipos[i]]);
          }
          ++nfirings;
          //cerr << "INFER " << OUT(infer) << endl;
          bool newly_inferred = facts.INSERT(infer).second;
          if (newly_inferred &&
              (r[3] == NULL || *r[3] == infer.parts[0]) &&
              (r[4] == NULL || *r[4] == infer.parts[1]) &&
              (r[5] == NULL || *r[5] == infer.parts[2])) {
            buffered.push_back(infer);
          }
        }
      }
      lit = lhs_range.second;
    }
    if (lit == lhs.end() && !buffered.empty()) {
      rhs.clear();
      deque<IDTrip>::const_iterator it = buffered.begin();
      for (; it != buffered.end(); ++it) {
        rhs.insert(pair<ID, IDTrip>(it->parts[j2-3], *it));
      }
      buffered.clear();
      lit = lhs.begin();
    }
  }

  MARK_TIME(end_time);

  unsigned long x[6];
  unsigned long y[6];
  x[0] = nprobes;
  x[1] = nmatches;
  x[2] = nfirings;
  x[3] = (unsigned long) (facts.size() - oldsize);
  x[4] = lhssize;
  x[5] = rhssize;
  MPI::COMM_WORLD.Reduce(x, y, 6, MPI::UNSIGNED_LONG, MPI::SUM, 0);
  if (MPI::COMM_WORLD.Get_rank() == 0) {
    cerr << "LHS: " << y[4] << endl;
    cerr << "RHS: " << y[5] << endl;
    cerr << "PROBES: " << y[0] << endl;
    cerr << "MATCHES: " << y[1] << endl;
    cerr << "FIRINGS: " << y[2] << endl;
    cerr << "INFERENCES: " << y[3] << endl;
    cerr << "[TIME] LHS INDEX: " << TIMEOUTPUT(DIFFTIME(lhsidx_end_time, begin_time)) << endl;
    cerr << "[TIME] RHS INDEX: " << TIMEOUTPUT(DIFFTIME(rhsidx_end_time, lhsidx_end_time)) << endl;
    cerr << "[TIME] FIRING: " << TIMEOUTPUT(DIFFTIME(end_time, rhsidx_end_time)) << endl;
  }
  return facts.size() - oldsize;
}

TIME_T(repl_end_time);

size_t rdfs_infer(Dict *dict, STORAGE<IDTrip> &facts) {

  int rank = MPI::COMM_WORLD.Get_rank();

  size_t origsize = facts.size();

  rdfs_replicate(dict, facts);

  MARK_TIME(repl_end_time);

  STORAGE<IDTrip> inferences;

  RDF_TERM(subprop, "<http://www.w3.org/2000/01/rdf-schema#subPropertyOf>");
  RDF_TERM(subclass, "<http://www.w3.org/2000/01/rdf-schema#subClassOf>");
  RDF_TERM(domain, "<http://www.w3.org/2000/01/rdf-schema#domain>");
  RDF_TERM(range, "<http://www.w3.org/2000/01/rdf-schema#range>");
  RDF_TERM(type, "<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>");

  ID type_id;
  if (!dict->lookup(type, type_id)) {
    cerr << "[ERROR] Unbelievably, rdf:type did not occur in the dataset." << endl;
    return 0;
  }

  size_t oldsize = facts.size();
  ID *r[9];
  ID id;
  ID zero = ID::zero();
  int ipos[3];

  // Inference

  if (dict->lookup(subprop, id)) {
    r[0] = NULL; r[1] = &id; r[2] = NULL;
    r[3] = NULL; r[4] = &id; r[5] = NULL;
    r[6] = NULL; r[7] = &id; r[8] = NULL;
    ipos[0] = 3; ipos[1] = 7; ipos[2] = 2;
    if (rank == 0) cerr << "=== SUBPROPERTY TRANSITIVITY ===" << endl;
    size_t ninfer = rule_infer(facts, r, 0, 5, ipos);
    r[0] = NULL; r[1] = &id; r[2] = NULL;
    r[3] = NULL; r[4] = NULL; r[5] = NULL;
    r[6] = NULL; r[7] = NULL; r[8] = NULL;
    ipos[0] = 3; ipos[1] = 2; ipos[2] = 5;
    if (rank == 0) cerr << "=== SUBPROPERTY ===" << endl;
    ninfer = rule_infer(facts, r, 0, 4, ipos);
  }

  if (dict->lookup(domain, id)) {
    r[0] = NULL; r[1] = &id; r[2] = NULL;
    r[3] = NULL; r[4] = NULL; r[5] = NULL;
    r[6] = NULL; r[7] = &type_id; r[8] = NULL;
    ipos[0] = 3; ipos[1] = 7; ipos[2] = 2;
    if (rank == 0) cerr << "=== DOMAIN ===" << endl;
    size_t ninfer = rule_infer(facts, r, 0, 4, ipos);
  }

  if (dict->lookup(range, id)) {
    r[0] = NULL; r[1] = &id; r[2] = NULL;
    r[3] = NULL; r[4] = NULL; r[5] = NULL;
    r[6] = NULL; r[7] = &type_id; r[8] = NULL;
    ipos[0] = 5; ipos[1] = 7; ipos[2] = 2;
    if (rank == 0) cerr << "=== RANGE ===" << endl;
    size_t ninfer = rule_infer(facts, r, 0, 4, ipos);
  }

  if (dict->lookup(subclass, id)) {
    r[0] = NULL; r[1] = &id; r[2] = NULL;
    r[3] = NULL; r[4] = &id; r[5] = NULL;
    r[6] = NULL; r[7] = &id; r[8] = NULL;
    ipos[0] = 3; ipos[1] = 7; ipos[2] = 2;
    if (rank == 0) cerr << "=== SUBCLASS TRANSITIVITY ===" << endl;
    size_t ninfer = rule_infer(facts, r, 0, 5, ipos);
    r[0] = NULL; r[1] = &id; r[2] = NULL;
    r[3] = NULL; r[4] = &type_id; r[5] = NULL;
    r[6] = NULL; r[7] = &type_id; r[8] = NULL;
    ipos[0] = 3; ipos[1] = 7; ipos[2] = 2;
    if (rank == 0) cerr << "=== SUBCLASS ===" << endl;
    ninfer = rule_infer(facts, r, 0, 5, ipos);
  }

  unsigned long n[4];
  n[0] = (unsigned long) oldsize;
  n[1] = (unsigned long) (facts.size() - oldsize);
  n[2] = (unsigned long) facts.size();
  n[3] = (unsigned long) origsize;
  unsigned long totaln[4];
  MPI::COMM_WORLD.Reduce(n, totaln, 4, MPI::UNSIGNED_LONG, MPI::SUM, 0);
  if (rank == 0) {
    cerr << "=== OVERALL ===" << endl;
    cerr << "FACTS: " << totaln[3] << endl;
    cerr << "REDIST: " << totaln[0] << endl;
    cerr << "INFERENCES: " << totaln[1] << endl;
    cerr << "TOTAL: " << totaln[2] << endl;
  }
  return facts.size() - oldsize;
}

void write(const char *filename, Dict *dict, const STORAGE<IDTrip> &trips) {
  OutputStream *os;
  NEW(os, MPIDistPtrFileOutputStream, MPI::COMM_WORLD, filename,
      MPI::MODE_WRONLY | MPI::MODE_CREATE, MPI::INFO_NULL, PAGEBYTES, true);
  NTriplesWriter *ntw;
  NEW(ntw, NTriplesWriter, os);
  STORAGE<IDTrip>::const_iterator it = trips.begin();
  for (; it != trips.end(); ++it) {
    RDFTerm subj = dict->decode(it->parts[0]);
    if (subj.isLiteral()) {
      continue;
    }
    RDFTerm pred = dict->decode(it->parts[1]);
    if (pred.getType() != IRI) {
      continue;
    }
    RDFTerm obj = dict->decode(it->parts[2]);
    RDFTriple triple(subj, pred, obj);
    ntw->write(triple);
  }
  ntw->close();
  DELETE(ntw);
}

int main (int argc, char **argv) {
  TIME_T(load_begin_time, load_end_time);
  TIME_T(infer_begin_time, infer_end_time);
  TIME_T(write_begin_time, write_end_time);
  TIME_T(overall_begin_time, overall_end_time);

  MPI::Init(argc, argv);

  int rank = MPI::COMM_WORLD.Get_rank();
  int nproc = MPI::COMM_WORLD.Get_size();

  MARK_TIME(overall_begin_time);

  srand(rank);
  int r = rand();
  srand(time(NULL) ^ r);

  if (argc <= 1) {
    MPI::Finalize();
    return 1;
  }

  STORAGE<IDTrip> trips;

  MARK_TIME(load_begin_time);
  Dict *dict = load(argv[1], trips);
  MARK_TIME(load_end_time);

  GLOBAL_DICT = dict;

  MARK_TIME(infer_begin_time);
  rdfs_infer(dict, trips);
  MARK_TIME(infer_end_time);

  if (argc > 2) {
    MARK_TIME(write_begin_time);
    write(argv[2], dict, trips);
    MARK_TIME(write_end_time);
  }

#ifdef TOSTDOUT 
  int nothing;
  if (rank > 0) {
    MPI::COMM_WORLD.Recv(&nothing, 1, MPI::INT, rank - 1, 11);
  }
  STORAGE<IDTrip>::const_iterator it = trips.begin();
  for (; it != trips.end(); ++it) {
    int i;
    for (i = 0; i < 3; ++i) {
      cout << dict->decode(it->parts[i]) << " ";
    }
    cout << ".\n";
  }
  if (rank < nproc - 1) {
    MPI::COMM_WORLD.Send(&nothing, 1, MPI::INT, rank + 1, 11);
  }
#endif

  DELETE(dict);
  ASSERTNPTR(0);

  MARK_TIME(overall_end_time);

#if TIMING_USE != TIMING_NONE
  if (rank == 0) {
    cerr << "[TIME] LOAD: " << TIMEOUTPUT(DIFFTIME(load_end_time, load_begin_time)) << endl;
    cerr << "[TIME] DISTRIBUTE: " << TIMEOUTPUT(DIFFTIME(repl_end_time, infer_begin_time)) << endl;
    cerr << "[TIME] INFER: " << TIMEOUTPUT(DIFFTIME(infer_end_time, repl_end_time)) << endl;
    cerr << "[TIME] WRITE: " << TIMEOUTPUT(DIFFTIME(write_end_time, write_begin_time)) << endl;
    cerr << "[TIME] OVERALL: " << TIMEOUTPUT(DIFFTIME(overall_end_time, overall_begin_time)) << endl;
  }
#endif

  MPI::Finalize();
}
