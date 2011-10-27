#ifndef __SIZEUNKNOWNEXCEPTION_H__
#define __SIZEUNKNOWNEXCEPTION_H__

#include <cstring>
#include <new>
#include "ex/TraceableException.h"

namespace ptr {

using namespace ex;
using namespace std;

class SizeUnknownException : public TraceableException {
public:
  SizeUnknownException(const char *file, const unsigned int line) throw();
  virtual ~SizeUnknownException() throw();
};

}

#endif /* __SIZEUNKNOWNEXCEPTION_H__ */
