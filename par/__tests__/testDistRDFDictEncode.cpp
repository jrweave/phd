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

#include "par/__tests__/unit4mpi.h"
#include "par/DistRDFDictEncode.h"

#include <deque>
#include <set>
#include "io/InputStream.h"
#include "io/OutputStream.h"
#include "par/DistComputation.h"
#include "par/Distributor.h"
#include "par/MPIDelimFileInputStream.h"
#include "par/MPIPacketDistributor.h"
#include "par/StringDistributor.h"
#include "rdf/NTriplesReader.h"
#include "rdf/RDFDictionary.h"
#include "rdf/RDFReader.h"
#include "rdf/RDFTriple.h"

#ifdef TESTFILE
#ifndef NUMLINES
#error "When specifying TESTFILE, must also specify NUMLINES."
#endif
#endif

#ifdef NUMLINES
#ifndef TESTFILE
#error "When specifying NUMLINES, must also specify TESTFILE."
#endif
#endif

#ifndef TESTFILE
#define TESTFILE "foaf_50.nt"
#define NUMLINES 50
#endif

#ifndef COORDEVERY
#define COORDEVERY 100000
#endif

#ifndef NUMREQUESTS
#define NUMREQUESTS 4
#endif

#ifndef PACKETBYTES
#define PACKETBYTES 128
#endif

#ifndef PAGEBYTES
#define PAGEBYTES 1
#endif

#ifndef IDBYTES
#define IDBYTES 8
#endif

#define STORAGE set
#define RDFSTORAGE set<RDFTriple, bool(*)(const RDFTriple &, const RDFTriple &)>
#define DECLARE_RDFSTORAGE(s) RDFSTORAGE s(RDFTriple::cmplt0)
#define INSERT(s,x) ((s).insert(x))
#define CONTAINS(s,x) ((s).count(x) > 0)

using namespace io;
using namespace par;
using namespace rdf;
using namespace std;

typedef RDFID<IDBYTES> ID;
typedef RDFDictionary<ID> Dict;
typedef struct {
  ID subj, pred, obj;
} IDTrip;

bool operator<(const IDTrip &i1, const IDTrip &i2) {
  return i1.subj < i2.subj ||
         (i1.subj == i2.subj && i1.pred <  i2.pred) ||
         (i1.subj == i2.subj && i1.pred == i2.pred && i1.obj < i2.obj);
}

class SetLoader : public OutputStream {
private:
  STORAGE<IDTrip> *trips;
public:
  SetLoader(STORAGE<IDTrip> &s) : trips(&s) {}
  ~SetLoader() throw(IOException) {}
  void close() throw(IOException) {}
  void flush() throw(IOException) {}
  void write(DPtr<uint8_t> *buf, size_t &nwritten)
      throw(IOException, SizeUnknownException, BaseException<void*>) {
    IDTrip trip;
    const uint8_t *mark = buf->dptr();
    memcpy(&(trip.subj), mark, IDBYTES);
    mark += IDBYTES;
    memcpy(&(trip.pred), mark, IDBYTES);
    mark += IDBYTES;
    memcpy(&(trip.obj), mark, IDBYTES);
    INSERT(*trips, trip);
    nwritten = buf->size();
  }
};

void load(const char *filename, RDFSTORAGE &store) {
  InputStream *is;
  NEW(is, MPIDelimFileInputStream, MPI::COMM_WORLD, filename,
      MPI::MODE_RDONLY, MPI::INFO_NULL, PAGEBYTES, to_ascii('\n'));
  RDFReader *rr;
  NEW(rr, NTriplesReader, is);
  RDFTriple triple;
  while (rr->read(triple)) {
    INSERT(store, triple);
  }
  rr->close();
  DELETE(rr);
}

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
  NEW(out, SetLoader, store);
  NEW(distcomp, DistRDFDictEncode<IDBYTES>, rank, size, rr, dist, out);
  distcomp->exec();
  Dict *dict = distcomp->getDictionary();
  DELETE(distcomp);
  return dict;
}

bool test() {
#if !CACHE_LOOKUPS
#warning "ARBITRARILY PASSING TEST BECAUSE CACHE_LOOKUPS IS NOT DEFINED, BUT IT MEANS NOTHING!  COME BACK AND PUT IN A REAL TEST CASE!"
  PASS;
#else
  try {
    DECLARE_RDFSTORAGE(triples);
    STORAGE<IDTrip> trips;
    load(TESTFILE, triples);
    Dict *dict = load(TESTFILE, trips);
    STORAGE<IDTrip>::iterator it = trips.begin();
    bool passing = true;
    for (; passing && it != trips.end(); ++it) {
      RDFTriple t (dict->decode(it->subj), dict->decode(it->pred), dict->decode(it->obj));
      passing = CONTAINS(triples, t);
    }
    PROG(passing);
    RDFSTORAGE::iterator tit = triples.begin();
    for (; passing && tit != triples.end(); ++tit) {
      IDTrip t;
      if (!dict->lookup(tit->getSubj(), t.subj) ||
          !dict->lookup(tit->getPred(), t.pred) ||
          !dict->lookup(tit->getObj(), t.obj)) {
        passing = false;
      } else {
        passing = CONTAINS(trips, t);
      }
    }
    PROG(passing);
    unsigned long sz = trips.size();
    unsigned long total_size;
    MPI::COMM_WORLD.Allreduce(&sz, &total_size, 1, MPI::INT, MPI::SUM);
    PROG(total_size == NUMLINES);
    DELETE(dict);
  } catch(TraceableException &e) {
    cerr << e.what() << endl;
    FAIL;
  }
  PASS;
#endif
}

int main (int argc, char **argv) {
  INIT(argc, argv);
  if (MPI::COMM_WORLD.Get_rank() == 0) {
    cerr << "[INFO] TESTFILE is " << TESTFILE << "; expecting " << NUMLINES << " lines." << endl;
  }
  TEST(test);
  FINAL;
}
