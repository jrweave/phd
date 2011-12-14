#ifndef __IRI__IRIREF_H__
#define __IRI__IRIREF_H__

#include <vector>
#include "iri/MalformedIRIRefException.h"
#include "ptr/BadAllocException.h"
#include "ptr/DPtr.h"
#include "sys/ints.h"
#include "ucs/UCSIter.h"

#define IRI_CHAR_EXCLAMATION_MARK UINT32_C(0x21)
#define IRI_CHAR_HASH UINT32_C(0x23)
#define IRI_CHAR_DOLLAR_SIGN UINT32_C(0x24)
#define IRI_CHAR_PERCENT UINT32_C(0x25)
#define IRI_CHAR_AMPERSAND UINT32_C(0x26)
#define IRI_CHAR_APOSTROPHE UINT32_C(0x27)
#define IRI_CHAR_LEFT_PARENTHESIS UINT32_C(0x28)
#define IRI_CHAR_RIGHT_PARENTHESIS UINT32_C(0x29)
#define IRI_CHAR_ASTERISK UINT32_C(0x2A)
#define IRI_CHAR_PLUS UINT32_C(0x2B)
#define IRI_CHAR_COMMA UINT32_C(0x2C)
#define IRI_CHAR_HYPHEN UINT32_C(0x2D)
#define IRI_CHAR_PERIOD UINT32_C(0x2E)
#define IRI_CHAR_SLASH UINT32_C(0x2F)
#define IRI_CHAR_ZERO UINT32_C(0x30)
#define IRI_CHAR_ONE UINT32_C(0x31)
#define IRI_CHAR_TWO UINT32_C(0x32)
#define IRI_CHAR_FOUR UINT32_C(0x34)
#define IRI_CHAR_FIVE UINT32_C(0x35)
#define IRI_CHAR_COLON UINT32_C(0x3A)
#define IRI_CHAR_SEMICOLON UINT32_C(0x3B)
#define IRI_CHAR_EQUALS UINT32_C(0x3D)
#define IRI_CHAR_QUESTION_MARK UINT32_C(0x3F)
#define IRI_CHAR_AT UINT32_C(0x40)
#define IRI_CHAR_UPPERCASE_A UINT32_C(0x41)
#define IRI_CHAR_LEFT_SQUARE_BRACKET UINT32_C(0x5B)
#define IRI_CHAR_RIGHT_SQUARE_BRACKET UINT32_C(0x5D)
#define IRI_CHAR_UNDERSCORE UINT32_C(0x5F)
#define IRI_CHAR_LOWERCASE_A UINT32_C(0x61)
#define IRI_CHAR_LOWERCASE_V UINT32_C(0x76)
#define IRI_CHAR_TILDE UINT32_C(0x7E)

#define IRI_CHAR_IS_DIGIT(c) (UINT32_C(0x30) <= (c) && (c) <= UINT32_C(0x39))
#define IRI_CHAR_IS_UPPERCASE_ALPHA(c) (UINT32_C(0x41) <= (c) && (c) <= UINT32_C(0x5A))
#define IRI_CHAR_IS_LOWERCASE_ALPHA(c) (UINT32_C(0x61) <= (c) && (c) <= UINT32_C(0x7A))
#define IRI_CHAR_IS_ALPHA(c) (IRI_CHAR_IS_LOWERCASE_ALPHA(c) || IRI_CHAR_IS_UPPERCASE_ALPHA(c))
#define IRI_CHAR_IS_HEXDIG(c) (IRI_CHAR_IS_DIGIT(c) || (UINT32_C(0x41) <= (c) && (c) <= UINT32_C(0x46)) || (UINT32_C(0x61) <= (c) && (c) <= UINT32_C(0x66)))

#define IRI_HEX_VALUE(c) (IRI_CHAR_IS_DIGIT(c) ? (c) - IRI_CHAR_ZERO : (IRI_CHAR_IS_UPPERCASE_ALPHA(c) ? (c) - IRI_CHAR_UPPERCASE_A + 10 : (IRI_CHAR_IS_LOWERCASE_ALPHA(c) ? (c) - IRI_CHAR_LOWERCASE_A + 10 : UINT32_C(16))))

namespace iri {

using namespace ptr;
using namespace std;
using namespace ucs;

template<typename iter>
bool isIRIReference(iter begin, iter end);

template<typename iter>
bool isIRI(iter begin, iter end);

template<typename iter>
bool isIRelativeRef(iter begin, iter end);

template<typename iter>
bool isScheme(iter begin, iter end);

template<typename iter>
bool isIHierPart(iter begin, iter end);

template<typename iter>
bool isIRelativePart(iter begin, iter end);

template<typename iter>
bool isIAuthority(iter begin, iter end);

template<typename iter>
bool isIUserInfo(iter begin, iter end);

template<typename iter>
bool isIHost(iter begin, iter end);

template<typename iter>
bool isIPLiteral(iter begin, iter end);

template<typename iter>
bool isIPv4Address(iter begin, iter end);

template<typename iter>
bool isIPv6Address(iter begin, iter end);

template<typename iter>
bool isIPvFutureAddress(iter begin, iter end);

template<typename iter>
bool isIRegName(iter begin, iter end);

template<typename iter>
bool isPort(iter begin, iter end);

template<typename iter>
bool isIPath(iter begin, iter end);

template<typename iter>
bool isIPathAbsEmpty(iter begin, iter end);

template<typename iter>
bool isIPathAbsolute(iter begin, iter end);

template<typename iter>
bool isIPathNoScheme(iter begin, iter end);

template<typename iter>
bool isIPathRootless(iter begin, iter end);

template<typename iter>
bool isIPathEmpty(iter begin, iter end);

template<typename iter>
bool isISegment(iter begin, iter end);

template<typename iter>
bool isISegmentNZ(iter begin, iter end);

template<typename iter>
bool isISegmentNZNC(iter begin, iter end);

template<typename iter>
bool isIQuery(iter begin, iter end);

template<typename iter>
bool isIFragment(iter begin, iter end);

template<typename iter>
bool isPctEncoded(iter begin, iter end);

class IRIRef;

enum IRIRefPart {
  SCHEME, USER_INFO, HOST, PORT, PATH, QUERY, FRAGMENT
};

class IRIRef {
protected:
  DPtr<uint8_t> *utf8str;
  bool normalized;
public:
  IRIRef() throw(BadAllocException);
      // <> relative IRI reference with empty path
  IRIRef(DPtr<uint8_t> *utf8str)
      throw(SizeUnknownException, MalformedIRIRefException);
  IRIRef(const IRIRef &iri) throw();
  IRIRef(const IRIRef *iri) throw();
  virtual ~IRIRef() throw();

  // Static Methods
  static bool isIPChar(const uint32_t codepoint) throw();
  static bool isReserved(const uint32_t codepoint) throw();
  static bool isUnreserved(const uint32_t codepoint) throw();
  static bool isIUnreserved(const uint32_t codepoint) throw();
  static bool isUCSChar(const uint32_t codepoint) throw();
  static bool isSubDelim(const uint32_t codepoint) throw();
  static bool isGenDelim(const uint32_t codepoint) throw();
  static bool isIPrivate(const uint32_t codepoint) throw();

  // Final Methods
  DPtr<uint8_t> *getUTF8String() const throw();

  bool isRelativeRef() const throw();
  bool isIRI() const throw();
  bool isAbsoluteIRI() const throw();

  DPtr<uint8_t> *getPart(const enum IRIRefPart part) const throw();

  // Virtual Methods
  virtual IRIRef *normalize() throw(BadAllocException);
  virtual IRIRef *resolve(IRIRef *base) throw(BadAllocException);

  // Operators
  IRIRef &operator=(const IRIRef &rhs) throw();
  bool operator==(const IRIRef &rhs) throw();
  bool operator!=(const IRIRef &rhs) throw();
  bool operator<(const IRIRef &rhs) throw();
  bool operator<=(const IRIRef &rhs) throw();
  bool operator>(const IRIRef &rhs) throw();
  bool operator>=(const IRIRef &rhs) throw();
};

}

#include "iri/IRIRef-inl.h"

#endif /* __IRI__IRIREF_H__ */
