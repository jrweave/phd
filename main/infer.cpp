#include <deque>
#include <set>
#include "io/InputStream.h"
#include "io/OutputStream.h"
#include "par/DistComputation.h"
#include "par/DistRDFDictEncode.h"
#include "par/Distributor.h"
#include "par/MPIDelimFileInputStream.h"
#include "par/MPIPacketDistributor.h"
#include "par/StringDistributor.h"
#include "ptr/alloc.h"
#include "ptr/DPtr.h"
#include "ptr/MPtr.h"
#include "rdf/NTriplesReader.h"
#include "rdf/RDFReader.h"
#include "rdf/RDFDictionary.h"

#define COORDEVERY 100000
#define NUMREQUESTS 4
#define PACKETBYTES 128
#define PAGEBYTES 4096
#define IDBYTES 8

#define USING_SET 1

#if USING_SET
#define STORAGE set
#define INSERT insert
#define LOWER_BOUND(store, pattern) (store.lower_bound(pattern))
#define UPPER_BOUND(store, pattern) (store.upper_bound(pattern))
#endif

using namespace io;
using namespace par;
using namespace rdf;
using namespace std;

typedef RDFID<IDBYTES> ID;
typedef RDFDictionary<ID> Dict;
typedef struct {
  ID parts[3];
} IDTrip;

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

//const ID idx_map(const int i,  const IDTrip &left, const IDTrip &right, ID *r[9]) {
//  switch (i) {
//    case 0: return left.subj;
//    case 1: return left.pred;
//    case 2: return left.obj;
//    case 3: return right.subj;
//    case 4: return right.pred;
//    case 5: return right.obj;
//    default: return *r[i];
//  }
//}

size_t replicate(STORAGE<IDTrip> &facts, ID *s, ID *p, ID *o) {
  size_t oldsize = facts.size();
  pair<IDTrip, IDTrip> rng = store_range(s, p, o);
  STORAGE<IDTrip>::iterator begin = LOWER_BOUND(facts, rng.first);
  STORAGE<IDTrip>::iterator end = UPPER_BOUND(facts, rng.second);
  deque<IDTrip> repls(begin, end);
  uint8_t *buf = NULL;
  unsigned long len;
  len = repls.size() * 3 * ID::size();
  if (!alloc(buf, len)) {
    cerr << "[ERROR] Processor " << MPI::COMM_WORLD.Get_rank() << " could not allocate " << len << " bytes." << endl;
    MPI::COMM_WORLD.Abort(-1);
  }
  deque<IDTrip>::iterator b, e;
  b = repls.begin();
  e = repls.end();
  uint8_t *ptr = buf;
  for (; b != e; ++b) {
    int i;
    for (i = 0; i < 3; ++i) {
      memcpy(ptr, b->parts[i].ptr(), ID::size());
      ptr += ID::size();
    }
  }
  int rank = MPI::COMM_WORLD.Get_rank();
  int size = MPI::COMM_WORLD.Get_size();
  int i;
  uint8_t *temp = NULL;
  for (i = 0; i < size; ++i) {
    if (i == rank) {
      MPI::COMM_WORLD.Bcast(&len, 1, MPI::UNSIGNED_LONG, i);
      MPI::COMM_WORLD.Bcast(buf, len, MPI::BYTE, i);
    } else {
      unsigned long sz;
      MPI::COMM_WORLD.Bcast(&sz, 1, MPI::UNSIGNED_LONG, i);
      if (!alloc(temp, sz)) {
        cerr << "[ERROR] Processor " << rank << " could not allocate " << sz << " bytes." << endl;
        MPI::COMM_WORLD.Abort(-1);
      }
      MPI::COMM_WORLD.Bcast(temp, sz, MPI::BYTE, i);
      unsigned long j;
      for (j = 0; j < sz; j += 3*ID::size()) {
        IDTrip trip;
        int k;
        for (k = 0; k < 3; ++k) {
          memcpy(trip.parts[k].ptr(), temp + j + k*ID::size(), ID::size());
        }
        facts.INSERT(trip);
      }
      dalloc(temp);
    }
  }
  cerr << "Line " << __LINE__ << " buf==" << (void*)buf << endl;
  dalloc(buf);
  cerr << "Line " << __LINE__ << endl;
  return facts.size() - oldsize;
}

size_t rule_infer(STORAGE<IDTrip> &facts, ID *r[9], const int j1, const int j2, const int ipos[3]) {
  size_t oldsize = facts.size();
  size_t nchecks = 0;
  size_t nmatches = 0;
  size_t nfires = 0;
  pair<IDTrip, IDTrip> store_rng = store_range(r[0], r[1], r[2]);
  STORAGE<IDTrip>::const_iterator store_begin, store_end;
  store_begin = LOWER_BOUND(facts, store_rng.first);
  store_end = UPPER_BOUND(facts, store_rng.second);
  set<IDTrip> index(store_begin, store_end);
  store_rng = store_range(r[3], r[4], r[5]);
  store_begin = LOWER_BOUND(facts, store_rng.first);
  store_end = UPPER_BOUND(facts, store_rng.second);
  list<IDTrip> matches(store_begin, store_end);
  nmatches = matches.size();
  list<IDTrip>::iterator match = matches.begin();
  while (match != matches.end()) {
    ++nchecks;
    const ID *tmp[3];
    int i;
    for (i = 0; i < 3; ++i) {
      if (i == j1) {
        tmp[i] = &(match->parts[j2-3]);
      } else {
        tmp[i] = r[i];
      }
    }
    pair<IDTrip, IDTrip> index_rng = store_range(tmp);
    set<IDTrip>::iterator index_begin, index_end;
    index_begin = index.lower_bound(index_rng.first);
    index_end = index.upper_bound(index_rng.second);
    for (; index_begin != index_end; ++index_begin) {
      ++nmatches;
      IDTrip infer;
      for (i = 0; i < 3; ++i) {
        infer.parts[i] = ipos[i] < 3 ? index_begin->parts[ipos[i]] :
                        (ipos[i] < 6 ? match->parts[ipos[i] - 3] :
                                       *r[ipos[i] - 6]);
      }
      if ((r[3] == NULL || *r[3] == infer.parts[0]) &&
          (r[4] == NULL || *r[4] == infer.parts[1]) &&
          (r[5] == NULL || *r[5] == infer.parts[2]) &&
          facts.count(infer) <= 0) {
        matches.push_back(infer);
      }
      facts.INSERT(infer);
      ++nfires;
    }
    list<IDTrip>::iterator temp = match++;
    matches.erase(temp);
  }
  cerr << "CHECKS: " << nchecks << endl;
  cerr << "MATCHES: " << nmatches << endl;
  cerr << "FIRINGS: " << nfires << endl;
  return facts.size() - oldsize;
}

size_t old_rule_infer(STORAGE<IDTrip> &facts, ID *r[9], const int j1, const int j2, const int ipos[3]) {
  size_t oldsize = facts.size();
  size_t nchecks = 0;
  size_t nfires = 0;
  bool trans = (r[1] != NULL && r[4] != NULL && *r[1] == *r[4] && ((j1 == 2 && j2 == 3) || (j1 == 0 && j2 == 5)));
  pair<IDTrip, IDTrip> rng1 = store_range(r[0], r[1], r[2]);
  STORAGE<IDTrip> recent_facts, new_facts, inferences;
  STORAGE<IDTrip>::iterator begin, end;
  cerr << "Line: " << __LINE__ << endl;
  if (trans) {
    cerr << "Line: " << __LINE__ << endl;
    begin = LOWER_BOUND(facts, rng1.first);
    end = UPPER_BOUND(facts, rng1.second);
    recent_facts.insert(begin, end);
  }
  cerr << "Line: " << __LINE__ << endl;
  do {
    cerr << "Line: " << __LINE__ << endl;
    if (trans) {
      cerr << "Line: " << __LINE__ << endl;
      begin = LOWER_BOUND(recent_facts, rng1.first);
      end = UPPER_BOUND(recent_facts, rng1.second);
    } else {
      cerr << "Line: " << __LINE__ << endl;
      begin = LOWER_BOUND(facts, rng1.first);
      end = UPPER_BOUND(facts, rng1.second);
    }
    cerr << "Line: " << __LINE__ << endl;
    inferences.clear();
    for (; begin != end; ++begin) {
      cerr << "Line: " << __LINE__ << endl;
      const ID *tmp[3];
      int i;
      for (i = 0; i < 3; ++i) {
        cerr << "Line: " << __LINE__ << endl;
        if (i == j2 - 3) {
          cerr << "Line: " << __LINE__ << endl;
          tmp[i] = &begin->parts[j1];
        } else {
          cerr << "Line: " << __LINE__ << endl;
          tmp[i] = r[3+i];
        }
      }
      cerr << "Line: " << __LINE__ << endl;
      pair<IDTrip, IDTrip> rng2 = store_range(tmp[0], tmp[1], tmp[2]);
      STORAGE<IDTrip>::iterator b = LOWER_BOUND(facts, rng2.first);
      STORAGE<IDTrip>::iterator e = UPPER_BOUND(facts, rng2.second);
      cerr << "Line: " << __LINE__ << endl;
      for (; b != e; ++b) {
        cerr << "Line: " << __LINE__ << endl;
        IDTrip infer;
        for (i = 0; i < 3; ++i) {
          infer.parts[i] = ipos[i] < 3 ? begin->parts[ipos[i]] :
                          (ipos[i] < 6 ? b->parts[ipos[i]-3] :
                                         *r[ipos[i]]);
        }
        bool already;
        if (facts.count(infer) <= 0 && (!trans || (new_facts.count(infer) <= 0 && recent_facts.count(infer) <= 0))) {
          cerr << "Line: " << __LINE__ << endl;
          if (inferences.INSERT(infer).second) {
            cerr << "Line: " << __LINE__ << endl;
            ++nfires;
          }
          cerr << "Line: " << __LINE__ << endl;
        }
        cerr << "Line: " << __LINE__ << endl;
        ++nchecks;
      }
    }
    cerr << "Line: " << __LINE__ << endl;
    if (trans) {
      cerr << "Line: " << __LINE__ << endl;
      new_facts.INSERT(recent_facts.begin(), recent_facts.end());
      recent_facts.swap(inferences);
    } else {
      cerr << "Line: " << __LINE__ << endl;
      facts.INSERT(inferences.begin(), inferences.end());
    }
    cerr << "Line: " << __LINE__ << endl;
  } while(!inferences.empty());
  cerr << "Line: " << __LINE__ << endl;
  if (trans) {
    cerr << "Line: " << __LINE__ << endl;
    facts.INSERT(recent_facts.begin(), recent_facts.end());
  }
  cerr << "Line: " << __LINE__ << endl;
  cerr << "CHECKS: " << nchecks << endl;
  cerr << "FIRINGS: " << nfires << endl;
  return facts.size() - oldsize;
}

#define RDF_TERM(name, cstr) \
  DPtr<uint8_t> * name ## str; \
  NEW(name ## str, MPtr<uint8_t>, strlen(cstr)); \
  ascii_strcpy(name ## str->dptr(), cstr); \
  RDFTerm name = RDFTerm::parse(name ## str); \
  name ## str->drop()

size_t rdfs_infer(Dict *dict, STORAGE<IDTrip> &facts) {

  cerr << "rdfs_infer called" << endl;

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

  // Data distribution

  cerr << "Line " << __LINE__ << endl;
  if (dict->lookup(subprop, id)) {
    cerr << "Line " << __LINE__ << endl;
    replicate(facts, NULL, &id, NULL);
  } else {
    cerr << "Line " << __LINE__ << endl;
    replicate(facts, &zero, &zero, &zero);
  }

  cerr << "Line " << __LINE__ << endl;
  if (dict->lookup(domain, id)) {
    replicate(facts, NULL, &id, NULL);
  } else {
    replicate(facts, &zero, &zero, &zero);
  }

  cerr << "Line " << __LINE__ << endl;
  if (dict->lookup(range, id)) {
    replicate(facts, NULL, &id, NULL);
  } else {
    replicate(facts, &zero, &zero, &zero);
  }

  cerr << "Line " << __LINE__ << endl;
  if (dict->lookup(subclass, id)) {
    replicate(facts, NULL, &id, NULL);
  } else {
    replicate(facts, &zero, &zero, &zero);
  }


  // Inference

  if (dict->lookup(subprop, id)) {
    r[0] = NULL; r[1] = &id; r[2] = NULL;
    r[3] = NULL; r[4] = &id; r[5] = NULL;
    r[6] = NULL; r[7] = &id; r[8] = NULL;
    ipos[0] = 0; ipos[1] = 7; ipos[2] = 5;
    cerr << "=== SUBPROPERTY TRANSITIVITY ===" << endl;
    size_t ninfer = rule_infer(facts, r, 2, 3, ipos);
    cerr << "INFERENCES: " << ninfer << endl;
    r[0] = NULL; r[1] = &id; r[2] = NULL;
    r[3] = NULL; r[4] = NULL; r[5] = NULL;
    r[6] = NULL; r[7] = NULL; r[8] = NULL;
    ipos[0] = 3; ipos[1] = 2; ipos[2] = 5;
    cerr << "=== SUBPROPERTY ===" << endl;
    ninfer = rule_infer(facts, r, 0, 4, ipos);
    cerr << "INFERENCES: " << ninfer << endl;
  }

  if (dict->lookup(domain, id)) {
    r[0] = NULL; r[1] = &id; r[2] = NULL;
    r[3] = NULL; r[4] = NULL; r[5] = NULL;
    r[6] = NULL; r[7] = &type_id; r[8] = NULL;
    ipos[0] = 3; ipos[1] = 7; ipos[2] = 2;
    cerr << "=== DOMAIN ===" << endl;
    size_t ninfer = rule_infer(facts, r, 0, 4, ipos);
    cerr << "INFERENCES: " << ninfer << endl;
  }

  if (dict->lookup(range, id)) {
    r[0] = NULL; r[1] = &id; r[2] = NULL;
    r[3] = NULL; r[4] = NULL; r[5] = NULL;
    r[6] = NULL; r[7] = &type_id; r[8] = NULL;
    ipos[0] = 5; ipos[1] = 7; ipos[2] = 2;
    cerr << "=== DOMAIN ===" << endl;
    size_t ninfer = rule_infer(facts, r, 0, 4, ipos);
    cerr << "INFERENCES: " << ninfer << endl;
  }

  if (dict->lookup(subclass, id)) {
    r[0] = NULL; r[1] = &id; r[2] = NULL;
    r[3] = NULL; r[4] = &id; r[5] = NULL;
    r[6] = NULL; r[7] = &id; r[8] = NULL;
    ipos[0] = 0; ipos[1] = 7; ipos[2] = 5;
    cerr << "=== SUBCLASS TRANSITIVITY ===" << endl;
    size_t ninfer = rule_infer(facts, r, 2, 3, ipos);
    cerr << "INFERENCES: " << ninfer << endl;
    r[0] = NULL; r[1] = &id; r[2] = NULL;
    r[3] = NULL; r[4] = &type_id; r[5] = NULL;
    r[6] = NULL; r[7] = &type_id; r[8] = NULL;
    ipos[0] = 3; ipos[1] = 7; ipos[2] = 2;
    cerr << "=== SUBCLASS ===" << endl;
    ninfer = rule_infer(facts, r, 0, 5, ipos);
    cerr << "INFERENCES: " << ninfer << endl;
  }

  return facts.size() - oldsize;
}

int main (int argc, char **argv) {
  MPI::Init(argc, argv);
  STORAGE<IDTrip> trips;
  Dict *dict = load(argv[1], trips);

  rdfs_infer(dict, trips);

  DELETE(dict);
  ASSERTNPTR(0);
  MPI::Finalize();
}
