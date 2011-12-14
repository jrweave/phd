#ifndef __LANG__MALFORMEDLANGRANGEEXCEPTION_H__
#define __LANG__MALFORMEDLANGRANGEEXCEPTION_H__

#include <cstring>
#include "ex/TraceableException.h"
#include "ptr/DPtr.h"
#include "sys/ints.h"
#include "ucs/UCSIter.h"

namespace lang {

using namespace ex;
using namespace ptr;
using namespace std;
using namespace ucs;

class MalformedLangRangeException : public TraceableException {
public:
  MalformedLangRangeException(const char *file, const unsigned int line) throw();
  MalformedLangRangeException(const char *file, const unsigned int line, DPtr<uint8_t> *mal)
      throw();
  virtual ~MalformedLangRangeException() throw();

  // Inherited Methods
  const char *what() const throw();
};

}

#endif /* __LANG__MALFORMEDLANGRANGEEXCEPTION_H__ */
