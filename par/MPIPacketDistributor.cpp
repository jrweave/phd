#include "par/MPIPacketDistributor.h"

#include "ptr/APtr.h"
#include "ptr/MPtr.h"

namespace par {

MPIPacketDistributor::MPIPacketDistributor(MPI::Intracomm &comm,
                                           const size_t packet_size,
                                           const size_t num_requests,
                                           const size_t check_every,
                                           int tag)
    throw(BadAllocException)
    : Distributor(), comm(comm), net_send_recv(1),
      last_send_checked(num_requests - 1), last_recv_checked(num_requests - 1),
      packet_size(packet_size), num_requests(num_requests),
      no_more_sends(false), send_requests(NULL), recv_requests(NULL),
      send_buffers(NULL), recv_buffers(NULL), tag(tag),
      check_every(check_every), check_count(0) {
  try {
    NEW_ARRAY(this->send_requests, MPI::Request, num_requests);
    NEW_ARRAY(this->recv_requests, MPI::Request, num_requests);
    NEW_ARRAY(this->send_buffers, DPtr<uint8_t>*, num_requests);
    NEW_ARRAY(this->recv_buffers, DPtr<uint8_t>*, num_requests);
  } catch (bad_alloc &e) {
    if (this->send_requests != NULL) DELETE_ARRAY(this->send_requests);
    if (this->recv_requests != NULL) DELETE_ARRAY(this->recv_requests);
    if (this->send_buffers != NULL) DELETE_ARRAY(this->send_buffers);
    if (this->recv_buffers != NULL) DELETE_ARRAY(this->recv_buffers);
    BadAllocException ex (__FILE__, __LINE__);
    RETHROW(ex, "Cannot instantiate requests and buffers.");
  } catch (BadAllocException &e) {
    if (this->send_requests != NULL) DELETE_ARRAY(this->send_requests);
    if (this->recv_requests != NULL) DELETE_ARRAY(this->recv_requests);
    if (this->send_buffers != NULL) DELETE_ARRAY(this->send_buffers);
    if (this->recv_buffers != NULL) DELETE_ARRAY(this->recv_buffers);
    RETHROW(e, "Cannot instantiate requests and buffers.");
  }
  size_t i;
  for (i = 0; i < num_requests; ++i) {
    this->available.push_back(i);
    this->send_buffers[i] = NULL;
    this->recv_buffers[i] = NULL;
  }
}

MPIPacketDistributor::~MPIPacketDistributor() throw(DistException) {
  DELETE_ARRAY(this->send_requests);
  DELETE_ARRAY(this->recv_requests);
  DPtr<uint8_t> **p = this->send_buffers;
  DPtr<uint8_t> **end = p + this->num_requests;
  for (; p != end; ++p) {
    if (*p != NULL) {
      (*p)->drop();
    }
  }
  p = this->recv_buffers;
  end = p + this->num_requests;
  for (; p != end; ++p) {
    if (*p != NULL) {
      (*p)->drop();
    }
  }
  DELETE_ARRAY(this->send_buffers);
  DELETE_ARRAY(this->recv_buffers);
}

void MPIPacketDistributor::init() throw(DistException, BadAllocException) {
  size_t i;
  for (i = 0; i < this->num_requests; ++i) {
    try {
      NEW(this->recv_buffers[i], MPtr<uint8_t>,
          this->packet_size * sizeof(uint8_t));
    } catch (bad_alloc &e) {
      size_t j;
      for (j = 0; j < i; ++j) {
        this->recv_buffers[j]->drop();
      }
      THROW(BadAllocException, sizeof(MPtr<uint8_t>));
    } catch (BadAllocException &e) {
      size_t j;
      for (j = 0; j < i; ++j) {
        this->recv_buffers[j]->drop();
      }
      RETHROW(e, "Could not instantiation receive buffers.");
    }
  }
  for (i = 0; i < this->num_requests; ++i) {
    this->recv_requests[i] = this->comm.Irecv(this->recv_buffers[i]->dptr(),
                                              this->packet_size, MPI_BYTE,
                                              MPI::ANY_SOURCE, this->tag);
    this->recv_order.push_back(i);
  }
}

bool MPIPacketDistributor::send(const int rank, DPtr<uint8_t> *msg)
    throw(DistException) {
  if (this->no_more_sends) {
    THROW(DistException, "You said there were no more sends!");
  }
  if (this->available.empty()) {
    list<size_t>::iterator it = this->send_order.begin();
    while (it != this->send_order.end()) {
      if (this->send_requests[*it].Test()) {
        this->available.push_back(*it);
        this->send_buffers[*it]->drop();
        this->send_buffers[*it] = NULL;
        this->send_order.erase(it);
        break;
      }
    }
    if (it == this->send_order.end()) {
      return false;
    }
  }
  if (msg == NULL) {
    THROW(DistException, "Tried to send NULL message.");
  }
  if (!msg->sizeKnown()) {
    THROW(DistException, "Size of message is unknown.");
  }
  if (msg->size() != this->packet_size) {
    THROW(DistException, "Message is not the size of a single packet.");
  }
  if (!msg->standable()) {
    THROW(DistException, "Message DPtr must be standable.");
  }
  size_t use = this->available.front();
  this->available.pop_front();
  msg->hold();
  //msg = msg->stand();
  this->send_buffers[use] = msg;
  this->send_requests[use] = this->comm.Isend(msg->dptr(), msg->size(),
                                              MPI_BYTE, rank, this->tag);
  ++this->net_send_recv;
  this->send_order.push_back(use);
  return true;
}

DPtr<uint8_t> *MPIPacketDistributor::receive() throw(DistException) {
  list<size_t>::iterator it = this->recv_order.begin();
  for (; it != this->recv_order.end() &&
         !this->recv_requests[*it].Test(); ++it) {
    // find available receive request, if any
  }
  if (it == this->recv_order.end()) {
    return NULL;
  }
  --this->net_send_recv;
  size_t i = *it;
  this->recv_order.erase(it);
  DPtr<uint8_t> *buf = this->recv_buffers[i];
  buf->hold();
  buf = buf->stand();
  this->recv_requests[i] = this->comm.Irecv(this->recv_buffers[i]->dptr(),
                                            this->packet_size, MPI_BYTE,
                                            MPI::ANY_SOURCE, this->tag);
  this->recv_order.push_back(i);
  return buf;
}

void MPIPacketDistributor::noMoreSends() throw(DistException) {
  if (this->no_more_sends) {
    THROW(DistException, "You already said no more sends!");
  }
  this->no_more_sends = true;
  --this->net_send_recv;
}

bool MPIPacketDistributor::done() throw(DistException) {
  if (++this->check_count < this->check_every) {
    return false;
  }
  this->check_count = 0;
  int net;
  this->comm.Allreduce(&this->net_send_recv, &net, 1, MPI::INT, MPI::SUM);
  if (net != 0) {
    return false;
  }
  MPI::Request *req = this->send_requests;
  MPI::Request *end = req + this->num_requests;
  for (; req != end; ++req) {
    if (*req != MPI::REQUEST_NULL) {
      while (!req->Test()) {
        // wait for send to complete; should already be done
      }
    }
  }
  req = this->recv_requests;
  end = req + this->num_requests;
  for (; req != end; ++req) {
    req->Cancel();
    while (!req->Test()) {
      // wait for cancellation to complete
    }
  }
  return true;
}

}
