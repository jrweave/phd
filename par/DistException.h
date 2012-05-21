#ifndef __IO__DISTEXCEPTION_H__
#define __IO__DISTEXCEPTION_H__

#include "ex/TraceableException.h"

namespace par {

using namespace ex;
using namespace std;

class DistException : public TraceableException {
public:
  DistException(const char *file, const unsigned int line)
      throw(const char *);
  DistException(const char *file, const unsigned int line,
      const char *message) throw(const char *);
  DistException(const DistException *) throw();
  virtual ~DistException() throw();
};

}

#endif /* __IO__DISTEXCEPTION_H__ */
