#ifndef __LANG__MALFORMEDLANGTAGEXCEPTION_H__
#define __LANG__MALFORMEDLANGTAGEXCEPTION_H__

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

class MalformedLangTagException : public TraceableException {
public:
  MalformedLangTagException(const char *file, const unsigned int line) throw();
  MalformedLangTagException(const char *file, const unsigned int line, DPtr<uint8_t> *mal)
      throw();
  virtual ~MalformedLangTagException() throw();

  // Inherited Methods
  const char *what() const throw();
};

}

#endif /* __LANG__MALFORMEDLANGTAGEXCEPTION_H__ */
