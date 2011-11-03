#ifndef __INVALIDCODEPOINTEXCEPTION_H__
#define __INVALIDCODEPOINTEXCEPTION_H__

#include "ex/TraceableException.h"
#include "sys/ints.h"

namespace ucs {

using namespace ex;
using namespace std;

class InvalidCodepointException : public TraceableException {
private:
  const uint32_t codepoint;
public:
  InvalidCodepointException(const char *file, const unsigned int line,
    const uint32_t codepoint) throw();
  virtual ~InvalidCodepointException() throw();

  // Inherited Methods
  const char *what() const throw();
};

}

#endif /* __INVALIDCODEPOINTEXCEPTION_H__ */
