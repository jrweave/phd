#ifndef __UCS__UTF8ITER_H__
#define __UCS__UTF8ITER_H__

#include "ptr/DPtr.h"
#include "ptr/SizeUnknownException.h"
#include "sys/ints.h"
#include "ucs/UCSIter.h"

namespace ucs {

using namespace ptr;
using namespace std;

class UTF8Iter : public UCSIter {
private:
  DPtr<uint8_t> *utf8str;
  const uint8_t *reset_mark;
  const uint8_t *marker;
  uint32_t reset_value;
  uint32_t value;
public:
  UTF8Iter() throw(BadAllocException);
  UTF8Iter(DPtr<uint8_t> *utf8str) throw(SizeUnknownException);
  UTF8Iter(const UTF8Iter &copy);
  virtual ~UTF8Iter();

  // Static Methods
  static UTF8Iter *begin(DPtr<uint8_t> *utf8str);
  static UTF8Iter *end(DPtr<uint8_t> *utf8str);

  // Implemented Abstract Methods
  UCSIter *start();
  UCSIter *finish();
  uint32_t current();
  UCSIter *advance();
  bool more();
  void mark();
  void reset();

  // Operators
  UTF8Iter &operator=(UTF8Iter &rhs);
  bool operator==(UTF8Iter &rhs);
  bool operator!=(UTF8Iter &rhs);
  uint32_t operator*();
  UTF8Iter &operator++();
  UTF8Iter operator++(int);
};

}

#include "ucs/UTF8Iter-inl.h"

#endif /* __UCS__UTF8ITER_H__ */
