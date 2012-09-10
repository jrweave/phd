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
#include "par/MPIPacketDistributor.h"

#include <cstdlib>
#include "ptr/MPtr.h"

using namespace par;
using namespace ptr;
using namespace std;

bool test1() {
  int commsize = MPI::COMM_WORLD.Get_size();
  int rank = MPI::COMM_WORLD.Get_rank();
  int max_packets = 100000;

  // These parameters should be tweaked based on underlying system.
  int num_requests = commsize;
  int check_every = max_packets / 2;
  int print_every = 10000;

  MPIPacketDistributor dist(MPI::COMM_WORLD, sizeof(int), num_requests, check_every, 0);
  srand(rank);
  int num_packets = rand() % max_packets;
  ONEBYONE_START
  cerr << "[" << rank << "] Plan to send " << num_packets << " packets." << endl;
  ONEBYONE_END
  DPtr<uint8_t> *packet;
  NEW(packet, MPtr<uint8_t>, sizeof(int)/sizeof(uint8_t));
  dist.init();
  int num_received = 0;
  int i = 0;
  int count_loops = 0;
  for (; i < num_packets; ++i) {
    int send_to = rand() % MPI::COMM_WORLD.Get_size();
    int msg = rand();
    packet = packet->stand();
    memcpy(packet->dptr(), &msg, sizeof(int));
    do {
      DPtr<uint8_t> *recv_packet = dist.receive();
      if (recv_packet != NULL) {
        ++num_received;
        recv_packet->drop();
      }
      if (++count_loops >= print_every) {
        cerr << "[" << rank << "] Sent " << i << " out of " << num_packets << " packets.   Received " << num_received << " packets." << endl;
        count_loops = 0;
      }
    } while(!dist.done() && !dist.send(send_to, packet));
  }
  packet->drop();
  dist.noMoreSends();
  cerr << "[" << rank << "] Sent all " << num_packets << " packets." << endl;
  do {
    DPtr<uint8_t> *recv_packet = dist.receive();
    if (recv_packet != NULL) {
      ++num_received;
      recv_packet->drop();
    }
    if (++count_loops >= print_every) {
      cerr << "[" << rank << "] Sent all " << num_packets << " packets.  Received " << num_received <<  " packets." << endl;
      count_loops = 0;
    }
  } while (!dist.done());
  int total_sends = 0;
  int total_recvs = 0;
  MPI::COMM_WORLD.Allreduce(&num_packets, &total_sends, 1, MPI::INT, MPI::SUM);
  MPI::COMM_WORLD.Allreduce(&num_received, &total_recvs, 1, MPI::INT, MPI::SUM);
  PROG(total_sends == total_recvs);
  PASS;
}

int main(int argc, char **argv) {
  INIT(argc, argv);
  TEST(test1);
  FINAL;
}
