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

#include "par/StringDistributor.h"

#include "ptr/MPtr.h"

namespace par {

StringDistributor::StringDistributor(const int my_rank,
                                     const size_t packet_size,
                                     Distributor *dist)
    throw(BaseException<void*>, BaseException<size_t>)
    : Distributor(), dist(dist),
      recv_packets(RecvMap(headerlt, allocator<DPtr<uint8_t>*>())),
      packet_size(packet_size), my_rank(my_rank), id_counter(0),
      no_more_sends(false), really_no_more_sends(false) {
  if (dist == NULL) {
    THROW(BaseException<void*>, NULL, "dist must not be NULL.");
  }
  if (packet_size <= sizeof(size_t) + sizeof(int) + (sizeof(uint32_t) << 1)) {
    THROW(BaseException<size_t>, packet_size,
          "Specified packet size is smaller than necessary header.");
  }
}

StringDistributor::~StringDistributor() throw(DistException) {
  DELETE(this->dist);
}

void StringDistributor::init() throw(DistException, BadAllocException) {
  try {
    this->dist->init();
  } JUST_RETHROW(DistException, "Couldn't init underlying Distributor.")
    JUST_RETHROW(BadAllocException, "Couldn't init underlying Distributor.")
}

bool StringDistributor::send(const int rank, DPtr<uint8_t> *msg)
    throw(DistException) {
  if (this->no_more_sends) {
    THROW(DistException, "You already said no more sends!");
  }
  if (!this->send_packets.empty()) {
    SendList::iterator it = this->send_packets.begin();
    while (it != this->send_packets.end() &&
           this->dist->send(this->pending_send_rank, *it)) {
      (*it)->drop();
      *it = NULL;
      it = this->send_packets.erase(it);
    }
    if (!this->send_packets.empty()) {
      return false;
    }
  }
  if (msg == NULL) {
    THROW(DistException, "msg must not be NULL.");
  }
  if (!msg->sizeKnown()) {
    THROW(DistException, "Size of msg is unknown.");
  }
  if (msg->size() <= this->packet_size - sizeof(size_t)) {
    DPtr<uint8_t> *packet;
    try {
      NEW(packet, MPtr<uint8_t>, this->packet_size * sizeof(uint8_t));
    } catch (bad_alloc &e) {
      RETHROW_AS(DistException, e);
    } catch (BadAllocException &e) {
      RETHROW_AS(DistException, e);
    }
    size_t len = msg->size();
    memcpy(packet->dptr(), &len, sizeof(size_t));
    memcpy(packet->dptr() + sizeof(size_t), msg->dptr(), msg->size());
    if (this->dist->send(rank, packet)) {
      packet->drop();
    } else {
      this->pending_send_rank = rank;
      this->send_packets.push_back(packet);
    }
    return true;
  }
  size_t header_size = sizeof(size_t) + sizeof(int) + (sizeof(uint32_t) << 1);
  header head;
  head.nbytes = msg->size();
  head.sender = this->my_rank;
  head.id = this->id_counter++;
  head.order = UINT32_C(0);
  this->pending_send_rank = rank;
  const uint8_t *mark = msg->dptr();
  const uint8_t *end = mark + msg->size();
  while (mark != end) {
    DPtr<uint8_t> *packet;
    try {
      NEW(packet, MPtr<uint8_t>, this->packet_size * sizeof(uint8_t));
    } catch (bad_alloc &e) {
      SendList::iterator it = this->send_packets.begin();
      for (; it != this->send_packets.end(); ++it) {
        (*it)->drop();
      }
      RETHROW_AS(DistException, e);
    } catch (BadAllocException &e) {
      SendList::iterator it = this->send_packets.begin();
      for (; it != this->send_packets.end(); ++it) {
        (*it)->drop();
      }
      RETHROW_AS(DistException, e);
    }
    size_t payloadlen = min((size_t)(end - mark),
                            this->packet_size - header_size);
    uint8_t *write_to = packet->dptr();
    memcpy(write_to, &head.nbytes, sizeof(size_t));
    write_to += sizeof(size_t);
    memcpy(write_to, &head.sender, sizeof(int));
    write_to += sizeof(int);
    memcpy(write_to, &head.id, sizeof(uint32_t));
    write_to += sizeof(uint32_t);
    memcpy(write_to, &head.order, sizeof(uint32_t));
    write_to += sizeof(uint32_t);
    memcpy(write_to, mark, payloadlen*sizeof(uint8_t));
    if (this->dist->send(this->pending_send_rank, packet)) {
      packet->drop();
    } else {
      this->send_packets.push_back(packet);
    }
    mark += payloadlen;
    ++head.order;
  }
  return true;
}

DPtr<uint8_t> *StringDistributor::receive() throw(DistException) {
  DPtr<uint8_t> *packet = this->dist->receive();
  if (packet == NULL) {
    return NULL;
  }
  size_t header_size = sizeof(size_t) + sizeof(int) + (sizeof(uint32_t) << 1);
  header head;
  memcpy(&head.nbytes, packet->dptr(), sizeof(size_t));
  if (head.nbytes <= this->packet_size - sizeof(size_t)/sizeof(uint8_t)) {
    DPtr<uint8_t> *payload = packet->sub(sizeof(size_t)/sizeof(uint8_t), head.nbytes);
    packet->drop();
    return payload;
  }
  const uint8_t *read_from = packet->dptr() + sizeof(size_t);
  memcpy(&head.sender, read_from, sizeof(int));
  read_from += sizeof(int);
  memcpy(&head.id, read_from, sizeof(uint32_t));
  read_from += sizeof(uint32_t);
  memcpy(&head.order, read_from, sizeof(uint32_t));
  size_t payloadlen = this->packet_size - header_size;
  size_t npackets = head.nbytes / payloadlen;
  if (head.nbytes % payloadlen > 0) {
    ++npackets;
  }
  if (this->recv_packets.count(head) < npackets - 1) {
    this->recv_packets.insert(pair<header, DPtr<uint8_t>*>(head, packet));
    return NULL;
  }
  DPtr<uint8_t> *msg;
  try {
    NEW(msg, MPtr<uint8_t>, head.nbytes*sizeof(uint8_t));
  } catch (bad_alloc &e) {
    RETHROW_AS(DistException, e);
  } catch (BadAllocException &e) {
    RETHROW_AS(DistException, e);
  }
  size_t offset = head.order * payloadlen;
  memcpy(msg->dptr() + offset,
         packet->dptr() + header_size,
         min(payloadlen, msg->size() - offset));
  packet->drop();
  pair<RecvMap::iterator, RecvMap::iterator> range
      = this->recv_packets.equal_range(head);
  RecvMap::iterator it;
  for (it = range.first; it != range.second; ++it) {
    offset = it->first.order * payloadlen;
    packet = it->second;
    memcpy(msg->dptr() + offset,
           packet->dptr() + header_size,
           min(payloadlen, msg->size() - offset));
    packet->drop();
  }
  this->recv_packets.erase(range.first, range.second);
  return msg;
}

void StringDistributor::noMoreSends() throw(DistException) {
  if (this->no_more_sends) {
    THROW(DistException, "Already called noMoreSends()!");
  }
  this->no_more_sends = true;
  if (this->send_packets.empty()) {
    try {
      this->dist->noMoreSends();
      this->really_no_more_sends = true;
    } JUST_RETHROW(DistException, "Underlying Distributor problem.")
  }
}

bool StringDistributor::done() throw(DistException) {
  if (!this->send_packets.empty()) {
    SendList::iterator it = this->send_packets.begin();
    while (it != this->send_packets.end() &&
           this->dist->send(this->pending_send_rank, *it)) {
      (*it)->drop();
      *it = NULL;
      it = this->send_packets.erase(it);
    }
  }
  if (this->no_more_sends && this->send_packets.empty() &&
      !this->really_no_more_sends) {
    this->dist->noMoreSends();
    this->really_no_more_sends = true;
  }
  return this->dist->done();
}

bool StringDistributor::headerlt(const header &h1, const header &h2) {
  return h1.nbytes <  h2.nbytes ||
        (h1.nbytes == h2.nbytes && h1.sender <  h2.sender) ||
        (h1.nbytes == h2.nbytes && h1.sender == h2.sender && h1.id < h2.id);
}

}
