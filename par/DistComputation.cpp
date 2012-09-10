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

#include "par/DistComputation.h"

#include "ptr/MPtr.h"

namespace par {

using namespace ex;
using namespace ptr;
using namespace std;

DistComputation::DistComputation(Distributor *dist)
    throw(BaseException<void*>)
    : dist(dist) {
  if (dist == NULL) {
    THROW(BaseException<void*>, NULL, "dist must not be NULL.");
  }
}

DistComputation::~DistComputation() THROWS(DistException) {
  DELETE(this->dist);
}
TRACE(DistException, "Couldn't deconstruct.")

void DistComputation::exec()
    throw(DistException, BadAllocException, TraceableException) {
  DPtr<uint8_t> *buffer = NULL;
  DPtr<uint8_t> *recvd = NULL;
  DPtr<uint8_t> *msg = NULL;
  try {
    this->start();

    NEW(buffer, MPtr<uint8_t>, 1024);
    this->dist->init();

    size_t len;
    int send_to;
    do {
      send_to  = this->pickup(buffer, len);
      if (send_to < -1) {
        continue; // results in break
      }
      if (buffer == NULL) {
        THROW(TraceableException,
              "Call to pickup set buffer to NULL.");
      }
      if (!buffer->sizeKnown()) {
        THROW(TraceableException,
              "Call to pickup set buffer to unknown size.");
      }
      if (len > buffer->size()) {
        THROW(TraceableException,
              "Call to pickup return len longer than buffer.");
      }
      if (send_to >= 0 && len < buffer->size()) {
        msg = buffer->sub(0, len);
        // vvv This check helps prevent excess copying when
        // vvv possibly standing buffer later.
        if (buffer->size() - len >= 1024 ||
            ((float)len) / ((float)buffer->size()) < 0.75f) {
          msg = msg->stand();
        }
      } else {
        msg = buffer;
        msg->hold();
      }
      do {
        recvd = this->dist->receive();
        if (recvd != NULL) {
          this->dropoff(recvd);
          recvd->drop();
          recvd = NULL;
        }
      } while (!this->dist->done() &&
               send_to >= 0 &&
               !this->dist->send(send_to, msg));
      msg->drop();
      msg = NULL;
      if (!buffer->alone()) {
        buffer = buffer->stand();
      }
    } while (send_to >= -1);
    this->dist->noMoreSends();
    buffer->drop();
    buffer = NULL;
    do {
      recvd = this->dist->receive();
      if (recvd != NULL) {
        this->dropoff(recvd);
        recvd->drop();
        recvd = NULL;
      }
    } while (!this->dist->done());
    this->finish();
  } catch (bad_alloc &e) {
    if (buffer != NULL) buffer->drop();
    if (recvd != NULL) recvd->drop();
    if (msg != NULL) recvd->drop();
    this->fail();
    THROWX(BadAllocException);
  } catch (BadAllocException &e) {
    if (buffer != NULL) buffer->drop();
    if (recvd != NULL) recvd->drop();
    if (msg != NULL) recvd->drop();
    this->fail();
    RETHROW(e, "(rethrow)");
  } catch (DistException &e) {
    if (buffer != NULL) buffer->drop();
    if (recvd != NULL) recvd->drop();
    if (msg != NULL) recvd->drop();
    this->fail();
    RETHROW(e, "Problem with underlying Distributor.");
  } catch (TraceableException &e) {
    if (buffer != NULL) buffer->drop();
    if (recvd != NULL) recvd->drop();
    if (msg != NULL) recvd->drop();
    this->fail();
    RETHROW(e, "Problem with computation.");
  }
}

}
