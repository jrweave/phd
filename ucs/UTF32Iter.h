#ifndef __UCS__UTF32ITER_H__
#define __UCS__UTF32ITER_H__

#include "ptr/DPtr.h"
#include "ptr/SizeUnknownException.h"
#include "sys/ints.h"
#include "ucs/UCSIter.h"

namespace ucs {

using namespace ptr;
using namespace std;

class UTF32Iter : public UCSIter {
private:
  DPtr<uint32_t> *utf32str;
  const uint32_t *mark;
  uint32_t value;
  bool flip;
public:
  UTF32Iter(DPtr<uint32_t> *utf32str) throw(SizeUnknownException);
  UTF32Iter(const UTF32Iter &copy);
  virtual ~UTF32Iter();

  // Static Methods
  static UTF32Iter *begin(DPtr<uint32_t> *utf32str);
  static UTF32Iter *end(DPtr<uint32_t> *utf32str);

  // Implemented Abstract Methods
  virtual UCSIter *start();
  virtual UCSIter *finish();
  virtual uint32_t current();
  virtual UCSIter *advance();
  virtual bool more();

  // Operators
  UTF32Iter &operator=(UTF32Iter &rhs);
  bool operator==(UTF32Iter &rhs);
  bool operator!=(UTF32Iter &rhs);
  uint32_t operator*();
  UTF32Iter &operator++();
};

}

#endif /* __UCS__UTF32ITER_H__ */
