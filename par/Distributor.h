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

#ifndef __PAR__DISTRIBUTOR_H__
#define __PAR__DISTRIBUTOR_H__

#include "par/DistException.h"
#include "ptr/BadAllocException.h"
#include "ptr/DPtr.h"

/* Best way to use Distributor:

  while (more items) {
    item = get an item
    DPtr<uint8_t> *msg = generate message from item
    ranks = to which processors to assign the item
    for each rank in ranks {
      do {
        DPtr<uint8_t> *recv_msg = dist.receive();
        if (recv_msg != NULL) {
          handle received message
        }
      } while (!dist.done() && !dist.send(rank, msg));
    }
  }
  dist.noMoreSends();
  do {
    DPtr<uint8_t> *recv_msg = dist.receive();
    if (recv_msg != NULL) {
      handle received message
    }
  } while (!dist.done());
*/

namespace par {

using namespace ptr;

class Distributor {
public:
  virtual ~Distributor() throw(DistException);
  virtual void init() throw(DistException, BadAllocException) = 0;
  virtual bool send(const int rank, DPtr<uint8_t> *msg) throw(DistException) =0;
  virtual DPtr<uint8_t> *receive() throw(DistException) = 0;
  virtual void noMoreSends() throw(DistException) = 0;
  virtual bool done() throw(DistException) = 0;
};

inline
Distributor::~Distributor() throw(DistException) {
  // do nothing
}

}

#endif /* __PAR__DISTRIBUTOR_H__ */
