#ifndef __UCS__UTF16ITER_H__
#define __UCS__UTF16ITER_H__

#include "ptr/DPtr.h"
#include "ptr/SizeUnknownException.h"
#include "sys/ints.h"
#include "ucs/UCSIter.h"

namespace ucs {

using namespace ptr;
using namespace std;

class UTF16Iter : public UCSIter {
private:
  DPtr<uint16_t> *utf16str;
  const uint16_t *reset_mark;
  const uint16_t *marker;
  uint32_t reset_value;
  uint32_t value;
  bool flip;
public:
  UTF16Iter() throw(BadAllocException);
  UTF16Iter(DPtr<uint16_t> *utf16str) throw(SizeUnknownException);
  UTF16Iter(const UTF16Iter &copy);
  virtual ~UTF16Iter();

  // Static Methods
  static UTF16Iter *begin(DPtr<uint16_t> *utf16str);
  static UTF16Iter *end(DPtr<uint16_t> *utf16str);

  // Implemented Abstract Methods
  virtual UCSIter *start();
  virtual UCSIter *finish();
  virtual uint32_t current();
  virtual UCSIter *advance();
  virtual bool more();
  virtual void mark();
  virtual void reset();

  // Operators
  UTF16Iter &operator=(UTF16Iter &rhs);
  bool operator==(UTF16Iter &rhs);
  bool operator!=(UTF16Iter &rhs);
  uint32_t operator*();
  UTF16Iter &operator++();
  UTF16Iter operator++(int);
};

}

#endif /* __UCS__UTF16ITER_H__ */
