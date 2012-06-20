#ifndef __PAR__DISTCOMPUTATION_H__
#define __PAR__DISTCOMPUTATION_H__

#include "ex/BaseException.h"
#include "ex/TraceableException.h"
#include "par/DistException.h"
#include "par/Distributor.h"
#include "ptr/DPtr.h"
#include "sys/ints.h"

namespace par {

using namespace ex;
using namespace par;
using namespace ptr;
using namespace std;

class DistComputation {
private:
  Distributor *dist;
protected:
  DistComputation(Distributor *dist) throw(BaseException<void*>);

  virtual void start() throw(TraceableException) = 0;

  // Write message into buffer.  If buffer is not long enough,
  // call buffer->drop() and assign buffer a new DPtr<uint8_t> *
  // that is long enough.  The new buffer must have a known
  // size (buffer->sizeKnown() == true).  Assign to len the number
  // of bytes written to the buffer (which may be less than
  // buffer->size()).
  //
  // buffer is guaranteed to be writable by convention, i.e.,
  // buffer->alone() == true.  Therefore, there is no need
  // to check buffer->standable() or to call buffer->stand().
  // Follow the appropriate conventions of DPtr.  For example,
  // if you call hold() and keep reference, make sure to call
  // drop() when done with it.
  //
  // Return the rank of process to which to send the data. -1
  // means that there is currently no data to pickup, but there
  // may be later. < -1 means that there is no more data to pickup,
  // and so pickup no longer needs to be called.  pickup must
  // eventually return < -1 in order for the distributed
  // computation to complete.
  virtual int pickup(DPtr<uint8_t> *&buffer, size_t &len)
      throw(BadAllocException, TraceableException) = 0;

  // Treat msg with the appropriate DPtr conventions.
  virtual void dropoff(DPtr<uint8_t> *msg) throw(TraceableException) = 0;
 
  virtual void finish() throw(TraceableException) = 0;

  virtual void fail() throw() = 0;
 
public:
  virtual ~DistComputation() throw (DistException);

  void exec() throw(DistException, BadAllocException, TraceableException);
};

}

#endif /* __PAR__DISTCOMPUTATION_H__ */
