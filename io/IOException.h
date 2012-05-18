#ifndef __IO__IOEXCEPTION_H__
#define __IO__IOEXCEPTION_H__

#include "ex/TraceableException.h"

namespace io {

using namespace ex;
using namespace std;

class IOException : public TraceableException {
public:
  IOException(const char *file, const unsigned int line)
      throw(const char *);
  IOException(const char *file, const unsigned int line,
      const char *message) throw(const char *);
  IOException(const IOException *) throw();
  virtual ~IOException() throw();
};

}

#endif /* __IO__IOEXCEPTION_H__ */
