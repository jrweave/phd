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
#include "rdf/NTriplesReader.h"
#include "rdf/RDFReader.h"
#include "rdf/RDFDictionary.h"

#define COORDEVERY 100000
#define NUMREQUESTS 4
#define PACKETBYTES 128
#define PAGEBYTES 4096
#define IDBYTES 8

#define STORAGE deque
#define INSERT push_back

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
  NEW(out, SetLoader, store);
  NEW(distcomp, DistRDFDictEncode<IDBYTES>, rank, size, rr, dist, out);
  distcomp->exec();
  Dict *dict = distcomp->getDictionary();
  DELETE(distcomp);
  return dict;
}

int main (int argc, char **argv) {
  MPI::Init(argc, argv);
  STORAGE<IDTrip> trips;
  Dict *dict = load(argv[1], trips);
  STORAGE<IDTrip>::iterator it = trips.begin();
//  for (; it != trips.end(); ++it) {
//    cout << dict->decode(it->subj) << " " << dict->decode(it->pred) << " " << dict->decode(it->obj) << " ." << endl;
//  }
  cout << trips.size() << endl;
  DELETE(dict);
  MPI::Finalize();
}
