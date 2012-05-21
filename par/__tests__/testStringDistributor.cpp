#include "par/__tests__/unit4mpi.h"
#include "par/StringDistributor.h"

#include <cstdlib>
#include <sstream>
#include "par/MPIDelimFileInputStream.h"
#include "par/MPIPacketDistributor.h"
#include "rdf/RDFTriple.h"
#include "sys/char.h"

using namespace ex;
using namespace par;
using namespace ptr;
using namespace rdf;
using namespace std;

bool test(char *filename) {
  stringstream readbuf (stringstream::in | stringstream::out);
  stringstream recvbuf (stringstream::in | stringstream::out);
  int commsize = MPI::COMM_WORLD.Get_size();
  int rank = MPI::COMM_WORLD.Get_rank();

  // These parameters should be tweaked based on underlying system.
  int num_requests = commsize;
  int check_every = 25;
  size_t packet_size = 128;

  cerr << "[" << rank << "] Opening " << filename << endl;

  MPIDelimFileInputStream mis(MPI::COMM_WORLD, filename,
      MPI::MODE_RDONLY, MPI::INFO_NULL, 4096, to_ascii('\n'));

  MPI::COMM_WORLD.Barrier();
  cerr << "[" << rank << "] Creating packet distributor" << endl;


  MPIPacketDistributor *packet_dist;
  NEW(packet_dist, MPIPacketDistributor, MPI::COMM_WORLD, packet_size, num_requests, check_every, 1);
  MPI::COMM_WORLD.Barrier();
  cerr << "[" << rank << "] Creating string distributor" << endl;
  StringDistributor dist(rank, packet_size, packet_dist);
  dist.init();

  DPtr<uint8_t> *line = mis.readDelimited();
  int local_lines = 0;
  int local_bytes = 0;
  int recvd_lines = 0;
  int recvd_bytes = 0;
  size_t nerrs = 0;
  while (line != NULL) {
    cerr << "[" << rank << "] loop" << endl;
    const uint8_t *b = line->dptr();
    const uint8_t *e = b + line->size();
    for (; b != e; ++b) {
      readbuf << to_ascii(*b);
    }
    readbuf << endl;
    try {
      RDFTriple::parse(line);
    } catch (TraceableException &e) {
      cerr << "****************************************************************************************************************[" << rank << "] Exception: " << e.amendStackTrace(__FILE__, __LINE__).what() << endl;
      ++nerrs;
    }
    ++local_lines;
    local_bytes += line->size();
    int send_to = rand() % commsize;
    do {
      cerr << "[" << rank << "] before receive" << endl;
      DPtr<uint8_t> *recvd = dist.receive();
      cerr << "[" << rank << "] after receive" << endl;
      if (recvd != NULL) {
        const uint8_t *b = recvd->dptr();
        const uint8_t *e = b + recvd->size();
        for (; b != e; ++b) {
          recvbuf << to_ascii(*b);
        }
        recvbuf << endl;
        try {
          RDFTriple::parse(recvd);
        } catch (TraceableException &e) {
          cerr << "****************************************************************************************************************[" << rank << "] Exception: " << e.amendStackTrace(__FILE__, __LINE__).what() << endl;
          ++nerrs;
        }
        ++recvd_lines;
        recvd_bytes += recvd->size();
        recvd->drop();
      }
      cerr << "[" << rank << "] test done and send" << endl;
    } while (!dist.done() && !dist.send(send_to, line));
    line->drop();
    line = mis.readDelimited();
  }
  dist.noMoreSends();
  do {
    DPtr<uint8_t> *recvd = dist.receive();
    if (recvd != NULL) {
      const uint8_t *b = recvd->dptr();
      const uint8_t *e = b + recvd->size();
      for (; b != e; ++b) {
        recvbuf << to_ascii(*b);
      }
      recvbuf << endl;
      try {
        RDFTriple::parse(recvd);
      } catch (TraceableException &e) {
        cerr << "****************************************************************************************************************[" << rank << "] Exception: " << e.amendStackTrace(__FILE__, __LINE__).what() << endl;
        ++nerrs;
      }
      ++recvd_lines;
      recvd_bytes += recvd->size();
      recvd->drop();
    }
  } while (!dist.done());

//  cerr.flush();
//  MPI::COMM_WORLD.Barrier();
//  cerr.flush();
//  ONEBYONE_START
//  cerr.flush();
//  cerr << "========== PROCESSOR " << rank << " READ ==============" << endl;
//  cerr << readbuf.str() << endl;
//  cerr << "========== PROCESSOR " << rank << " RECEIVED ==========" << endl;
//  cerr << recvbuf.str() << endl;
//  cerr.flush();
//  ONEBYONE_END
//  cerr.flush();
//  MPI::COMM_WORLD.Barrier();

  int total_lines_read, total_lines_dist;
  int total_bytes_read, total_bytes_dist;
  MPI::COMM_WORLD.Allreduce(&local_lines, &total_lines_read, 1, MPI::INT, MPI::SUM);
  MPI::COMM_WORLD.Allreduce(&recvd_lines, &total_lines_dist, 1, MPI::INT, MPI::SUM);
  cerr << "read " << total_lines_read << "  dist " << total_lines_dist <<  "  lines" << endl;
  PROG(total_lines_read == total_lines_dist);
  MPI::COMM_WORLD.Allreduce(&local_bytes, &total_bytes_read, 1, MPI::INT, MPI::SUM);
  MPI::COMM_WORLD.Allreduce(&recvd_bytes, &total_bytes_dist, 1, MPI::INT, MPI::SUM);
  cerr << "read " << total_bytes_read << "  dist " << total_bytes_dist <<  "  bytes" << endl;
  PROG(total_bytes_read == total_bytes_dist);

  PROG(nerrs == 0); // moment of truth

  mis.close();
  PASS;
}

int main(int argc, char **argv) {
  INIT(argc, argv);
  TEST(test, argv[1]);
  FINAL;
}
