#ifndef __UTF_H__
#define __UTF_H__

#include "ex/TraceableException.h"
#include "ptr/BadAllocException.h"
#include "ptr/DPtr.h"
#include "ptr/SizeUnknownException.h"
#include "sys/ints.h"

namespace ucs {

using namespace ex;
using namespace ptr;
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

enum BOM {
  LITTLE, ANY, NONE, BIG
};

// Returns number of uint8_t
size_t utf8len(const uint32_t codepoint) throw(InvalidEncodingException);

size_t utf8len(const uint32_t codepoint, uint8_t *utf8val)
    throw(InvalidEncodingException);

DPtr<uint8_t> *utf8enc(DPtr<uint32_t> *codepoints)
    throw(SizeUnknownException, InvalidEncodingException, BadAllocException);

DPtr<uint32_t> *utf8dec(DPtr<uint8_t> *utf8str)
    throw(SizeUnknownException, InvalidEncodingException, BadAllocException);

size_t utf8nchars(const DPtr<uint8_t> *utf8str) throw(SizeUnknownException,
    InvalidEncodingException);

uint32_t utf8char(const uint8_t *utf8str) throw(InvalidEncodingException);

uint32_t utf8char(const uint8_t *utf8str, const uint8_t **next)
    throw(InvalidEncodingException);

// Returns number of uint16_t
size_t utf16len(const uint32_t codepoint, const enum BOM bom)
    throw(InvalidEncodingException);

size_t utf16len(const uint32_t codepoint, const enum BOM bom,
    uint16_t *utf16val) throw(InvalidEncodingException);

DPtr<uint16_t> *utf16enc(DPtr<uint32_t> *codepoints, const enum BOM bom)
    throw(SizeUnknownException, InvalidEncodingException, BadAllocException);

DPtr<uint32_t> *utf16dec(DPtr<uint16_t> *utf16str)
    throw(SizeUnknownException, InvalidEncodingException, BadAllocException);

size_t utf16nchars(const DPtr<uint16_t> *utf16str) throw(SizeUnknownException,
    InvalidEncodingException);

bool utf16flip(const DPtr<uint16_t> *utf16str) throw(SizeUnknownException);

bool utf16flip(const DPtr<uint16_t> *utf16str, const uint16_t **start)
    throw(SizeUnknownException);

uint32_t utf16char(const uint16_t *utf16str, const bool flip)
    throw(InvalidEncodingException);

uint32_t utf16char(const uint16_t *utf16str, const bool flip,
    const uint16_t **next) throw(InvalidEncodingException);

// Returns number of uint32_t
size_t utf32len(const uint32_t codepoint, const enum BOM bom)
    throw(InvalidEncodingException);

size_t utf32len(const uint32_t codepoint, const enum BOM bom,
    uint32_t *utf32val) throw(InvalidEncodingException);

DPtr<uint32_t> *utf32enc(DPtr<uint32_t> *codepoints, const enum BOM bom)
    throw(SizeUnknownException, InvalidEncodingException, BadAllocException);

DPtr<uint32_t> *utf32dec(DPtr<uint32_t> *utf32str)
    throw(SizeUnknownException, InvalidEncodingException, BadAllocException);

size_t utf32nchars(const DPtr<uint32_t> *utf32str) throw(SizeUnknownException,
    InvalidEncodingException);

bool utf32flip(const DPtr<uint32_t> *utf32str) throw(SizeUnknownException);

bool utf32flip(const DPtr<uint32_t> *utf32str, const uint32_t **start)
    throw(SizeUnknownException);

uint32_t utf32char(const uint32_t *utf32str, const bool flip)
    throw(InvalidEncodingException);

uint32_t utf32char(const uint32_t *utf32str, const bool flip,
    const uint32_t **next) throw(InvalidEncodingException);

}

#endif /* __UTF_H__ */
