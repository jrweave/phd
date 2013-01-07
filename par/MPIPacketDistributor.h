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

#ifndef __PAR__MPIPACKETDISTRIBUTOR_H__
#define __PAR__MPIPACKETDISTRIBUTOR_H__

#include <list>
#include <mpi.h>
#include "par/Distributor.h"

namespace par {

class MPIPacketDistributor : public Distributor {
private:
  MPI::Intracomm &comm;
  list<size_t> available;
  list<size_t> send_order;
  list<size_t> recv_order;
  MPI::Request *send_requests;
  MPI::Request *recv_requests;
  DPtr<uint8_t> **send_buffers;
  DPtr<uint8_t> **recv_buffers;
  int net_send_recv;
  size_t last_send_checked;
  size_t last_recv_checked;
  const size_t packet_size;
  const size_t num_requests;
  const size_t check_every;
  size_t check_count;
  int tag;
  bool no_more_sends;
  bool declared_done;
public:
  MPIPacketDistributor(MPI::Intracomm &comm, const size_t packet_size,
                       const size_t num_requests, const size_t check_every,
                       int tag)
      throw(BadAllocException);
  virtual ~MPIPacketDistributor() throw(DistException);

  virtual void init() throw(DistException, BadAllocException);
  virtual bool send(const int rank, DPtr<uint8_t> *msg) throw(DistException);
  virtual DPtr<uint8_t> *receive() throw(DistException);
  virtual void noMoreSends() throw(DistException);
  virtual bool done() throw(DistException);
};

}

#endif /* __PAR__MPIPACKETDISTRIBUTOR_H__ */
