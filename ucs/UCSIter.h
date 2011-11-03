#ifndef __UCS__UCSITER_H__
#define __UCS__UCSITER_H__

#include "sys/ints.h"

namespace ucs {

using namespace std;

class UCSIter {
protected:
  UCSIter();
public:
  virtual ~UCSIter();
  virtual UCSIter *start() = 0;
  virtual UCSIter *finish() = 0;
  virtual uint32_t current() = 0;
  virtual UCSIter *advance() = 0;
  virtual bool more() = 0;
  virtual uint32_t next();
};

}

#endif /* __UCS_UCSITER_H__ */
