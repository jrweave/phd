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

  MPIDelimFileInputStream mis(MPI::COMM_WORLD, filename,
      MPI::MODE_RDONLY, MPI::INFO_NULL, 4096, to_ascii('\n'));

  MPI::COMM_WORLD.Barrier();


  MPIPacketDistributor *packet_dist;
  NEW(packet_dist, MPIPacketDistributor, MPI::COMM_WORLD, packet_size, num_requests, check_every, 1);
  MPI::COMM_WORLD.Barrier();
  StringDistributor dist(rank, packet_size, packet_dist);
  dist.init();

  DPtr<uint8_t> *line = mis.readDelimited();
  int local_lines = 0;
  int local_bytes = 0;
  int recvd_lines = 0;
  int recvd_bytes = 0;
  size_t nerrs = 0;
  while (line != NULL) {
    const uint8_t *b = line->dptr();
    const uint8_t *e = b + line->size();
    for (; b != e; ++b) {
      readbuf << to_ascii(*b);
    }
    readbuf << endl;
    bool blank_line = line->size() == 0 || (line->size() == 1 && **line == to_ascii('\n'));
    if (!blank_line) {
      try {
        RDFTriple::parse(line);
      } catch (TraceableException &e) {
        cerr << "[" << rank << "] Exception: " << e.amendStackTrace(__FILE__, __LINE__).what() << endl;
        ++nerrs;
      }
    }
    ++local_lines;
    local_bytes += line->size();
    int send_to = rand() % commsize;
    do {
      DPtr<uint8_t> *recvd = dist.receive();
      if (recvd != NULL) {
        const uint8_t *b = recvd->dptr();
        const uint8_t *e = b + recvd->size();
        for (; b != e; ++b) {
          recvbuf << to_ascii(*b);
        }
        recvbuf << endl;
        blank_line = recvd->size() == 0 || (recvd->size() == 1 && **recvd == to_ascii('\n'));
        if (!blank_line) {
          try {
            RDFTriple::parse(recvd);
          } catch (TraceableException &e) {
            cerr << "[" << rank << "] Exception: " << e.amendStackTrace(__FILE__, __LINE__).what() << endl;
            ++nerrs;
          }
        }
        ++recvd_lines;
        recvd_bytes += recvd->size();
        recvd->drop();
      }
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
      bool blank_line = recvd->size() == 0 || (recvd->size() == 1 && **recvd == to_ascii('\n'));
      if (!blank_line) {
        try {
          RDFTriple::parse(recvd);
        } catch (TraceableException &e) {
          cerr << "[" << rank << "] Exception: " << e.amendStackTrace(__FILE__, __LINE__).what() << endl;
          ++nerrs;
        }
      }
      ++recvd_lines;
      recvd_bytes += recvd->size();
      recvd->drop();
    }
  } while (!dist.done());

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
