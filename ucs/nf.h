#ifndef __NF_H__
#define __NF_H__

#include "ex/TraceableException.h"
#include "ptr/BadAllocException.h"
#include "ptr/DPtr.h"
#include "ptr/SizeUnknownException.h"
#include "sys/ints.h"

namespace ucs {

using namespace ex;
using namespace ptr;
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

#include "ucs_arrays.h"

bool nfvalid(const uint32_t codepoint) throw();

DPtr<uint32_t> *nfd(const DPtr<uint32_t> *codepoints)
    throw (InvalidCodepointException, SizeUnknownException,
    BadAllocException);

#ifndef UCS_NO_K
DPtr<uint32_t> *nfkd(const DPtr<uint32_t> *codepoints)
    throw (InvalidCodepointException, SizeUnknownException,
    BadAllocException);
#endif

#ifndef UCS_NO_C

DPtr<uint32_t> *nfc(const DPtr<uint32_t> *codepoints)
    throw (InvalidCodepointException, SizeUnknownException,
    BadAllocException);

#ifndef UCS_NO_K
DPtr<uint32_t> *nfkc(const DPtr<uint32_t> *codepoints)
    throw (InvalidCodepointException, SizeUnknownException,
    BadAllocException);
#endif

#endif /* !defined(UCS_NO_C) */
}

#endif /* __NF_H__ */