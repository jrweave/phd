#ifndef __UCS__UCSITER_H__
#define __UCS__UCSITER_H__

#include "sys/ints.h"
#include "ucs/utf.h"

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
  virtual size_t current(uint8_t *utf8);
  virtual size_t current(uint16_t *utf16, const enum BOM bom);
  virtual size_t current(uint32_t *utf32, const enum BOM bom);
  virtual UCSIter *advance() = 0;
  virtual bool more() = 0;
  virtual uint32_t next();
  virtual size_t next(uint8_t *utf8);
  virtual size_t next(uint16_t *utf16, const enum BOM bom);
  virtual size_t next(uint32_t *utf32, const enum BOM bom);
  virtual void mark() = 0;
  virtual void reset() = 0;
};

}

#endif /* __UCS_UCSITER_H__ */
