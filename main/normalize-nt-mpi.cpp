#include <iostream>
#include <map>
#include <mpi.h>
#include <string>
#include "io/IFStream.h"
#include "io/OFStream.h"
#include "par/MPIDelimFileInputStream.h"
#include "par/MPIDistPtrFileOutputStream.h"
#include "rdf/NTriplesReader.h"
#include "rdf/NTriplesWriter.h"
#include "sys/char.h"

using namespace io;
using namespace par;
using namespace rdf;
using namespace std;

#define ONCE_BARRIER_BEGIN \
  MPI::COMM_WORLD.Barrier(); \
  if (rank == 0) {

#define ONCE_BARRIER_END \
  } \
  MPI::COMM_WORLD.Barrier();

int main (int argc, char **argv) {

  MPI::Init(argc, argv);
  int rank = MPI::COMM_WORLD.Get_rank();


  unsigned long numinput = 0;
  unsigned long numoutput = 0;
  unsigned long numnormalized = 0;
  unsigned long numerror = 0;

  ONCE_BARRIER_BEGIN
    cerr << "Executing with " << MPI::COMM_WORLD.Get_size() << " processors.\n";
  ONCE_BARRIER_END

  map<string, size_t> errors;
  int i;
  NTriplesWriter *ntw = NULL;
  for (i = 1; i < argc; ++i) {
    if (argv[i][0] == '-' && argv[i][1] == 'o' && argv[i][2] == '\0') {
      if (i < argc - 1) {
        ++i;
        OutputStream *ofs;

        ONCE_BARRIER_BEGIN
          cerr << "Writing to " << argv[i] << endl;
        ONCE_BARRIER_END

        if (ntw != NULL) {
          ntw->close();
          DELETE(ntw);
        }
        NEW(ofs, MPIDistPtrFileOutputStream, MPI::COMM_WORLD, argv[i], MPI::MODE_WRONLY | MPI::MODE_CREATE, MPI::INFO_NULL, 1024*1024, true);
        NEW(ntw, NTriplesWriter, ofs);
      }
      continue;
    }

    ONCE_BARRIER_BEGIN
      cerr << "Normalizing " << argv[i] << endl;
    ONCE_BARRIER_END

    InputStream *ifs;

    ONCE_BARRIER_BEGIN
      cerr << "Reading from " << argv[i] << endl;
    ONCE_BARRIER_END

    NEW(ifs, MPIDelimFileInputStream, MPI::COMM_WORLD, argv[i], MPI::MODE_RDONLY, MPI::INFO_NULL, 4096, to_ascii('\n'));
    NTriplesReader ntr(ifs);
    RDFTriple triple;
    for (;;) {
      try {
        if (!ntr.read(triple)) {
          ntr.close();
          break;
        }
        RDFTriple norm(triple);
        norm.normalize();
        if (!norm.equals(triple)) {
          //cerr << "Normalized: " << triple << "     --->     " << norm << endl;
          ++numnormalized;
        }
        if (ntw == NULL) {
          cout << norm;
        } else {
          ntw->write(norm);
        }
        ++numinput;
        ++numoutput;
      } catch (TraceableException &e) {
        string str(e.what());
        map<string, size_t>::iterator it = errors.find(str);
        if (it == errors.end()) {
          errors.insert(pair<string, size_t>(str, 1));
        } else {
          ++(it->second);
        }
        cerr << "[ERROR] " << e.what() << endl;
        ++numinput;
        ++numerror;
        continue;
      }
    }
  }
  if (ntw != NULL) {
    ntw->close();
    DELETE(ntw);
  }
  if (rank > 0) {
    int nothing;
    MPI::COMM_WORLD.Recv(&nothing, 1, MPI::INT, rank - 1, 18);
  }
  cerr << "===== ERROR SUMMARY =====" << endl;
  map<string, size_t>::iterator it = errors.begin();
  for (; it != errors.end(); ++it) {
    cerr << "\t[" << it->second << "] " << it->first;
  }
  if (rank < MPI::COMM_WORLD.Get_size() - 1) {
    int nothing;
    MPI::COMM_WORLD.Send(&nothing, 1, MPI::INT, rank + 1, 18);
  }
  unsigned long nums[4];
  nums[0] = numinput;
  nums[1] = numnormalized;
  nums[2] = numerror;
  nums[3] = numoutput;
  unsigned long totals[4];
  MPI::COMM_WORLD.Reduce(nums, totals, 4, MPI::UNSIGNED_LONG, MPI::SUM, 0);
  if (rank == 0) {
    cerr << "===== SUMMARY =====" << endl;
    cerr << "Input triples: " << totals[0] << endl;
    cerr << "Non-normalized triples: " << totals[1] << endl;
    cerr << "Malformed triples: " << totals[2] << endl;
    cerr << "Output triples: " << totals[3] << endl;
  }

  MPI::Finalize();
}
