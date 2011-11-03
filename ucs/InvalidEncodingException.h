#ifndef __UCS__INVALIDENCODINGEXCEPTION_H__
#define __UCS__INVALIDENCODINGEXCEPTION_H__

#include "ex/TraceableException.h"

namespace ucs {

using namespace ex;
using namespace std;

class InvalidEncodingException : public TraceableException {
private:
  const uint32_t encoded;
public:
  InvalidEncodingException(const char *file, const unsigned int line,
    const char *message, const uint32_t encoded) throw();
  virtual ~InvalidEncodingException() throw();

  // Inherited Methods
  const char *what() const throw();
};

}

#endif /* __UCS__INVALIDENCODINGEXCEPTION_H__ */
