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

bool nfvalid(DPtr<uint32_t> *codepoints) throw (SizeUnknownException);

uint8_t nfd_qc(const DPtr<uint32_t> *codepoints, size_t *pos)
    throw (InvalidCodepointException, SizeUnknownException);

DPtr<uint32_t> *nfd(DPtr<uint32_t> *codepoints)
    throw (InvalidCodepointException, SizeUnknownException,
    BadAllocException);

#ifndef UCS_NO_K
uint8_t nfkd_qc(const DPtr<uint32_t> *codepoints, size_t *pos)
    throw (InvalidCodepointException, SizeUnknownException);

DPtr<uint32_t> *nfkd(DPtr<uint32_t> *codepoints)
    throw (InvalidCodepointException, SizeUnknownException,
    BadAllocException);
#endif

#ifndef UCS_NO_C

uint8_t nfc_qc(const DPtr<uint32_t> *codepoints, size_t *pos)
    throw (InvalidCodepointException, SizeUnknownException);

DPtr<uint32_t> *nfc(DPtr<uint32_t> *codepoints)
    throw (InvalidCodepointException, SizeUnknownException,
    BadAllocException);

#ifndef UCS_NO_K
uint8_t nfkc_qc(const DPtr<uint32_t> *codepoints, size_t *pos)
    throw (InvalidCodepointException, SizeUnknownException);

DPtr<uint32_t> *nfkc(DPtr<uint32_t> *codepoints)
    throw (InvalidCodepointException, SizeUnknownException,
    BadAllocException);
#endif

#endif /* !defined(UCS_NO_C) */
}

#endif /* __NF_H__ */
