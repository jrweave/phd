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
