#ifndef __LANG__LANGTAG_H__
#define __LANG__LANGTAG_H__

#include "lang/MalformedLangTagException.h"
#include "ptr/DPtr.h"
#include "sys/ints.h"

#define LANG_CHAR_HYPHEN UINT8_C(0x2D)
#define LANG_CHAR_UPPERCASE_A UINT8_C(0x41)
#define LANG_CHAR_LOWERCASE_A UINT8_C(0x61)
#define LANG_CHAR_LOWERCASE_D UINT8_C(0x64)
#define LANG_CHAR_LOWERCASE_E UINT8_C(0x65)
#define LANG_CHAR_LOWERCASE_F UINT8_C(0x66)
#define LANG_CHAR_LOWERCASE_I UINT8_C(0x69)
#define LANG_CHAR_LOWERCASE_L UINT8_C(0x6C)
#define LANG_CHAR_LOWERCASE_T UINT8_C(0x74)
#define LANG_CHAR_LOWERCASE_U UINT8_C(0x75)

#define LANG_CHAR_IS_A(c) ((c) == UINT8_C(0x41) || (c) == UINT8_C(0x61))
#define LANG_CHAR_IS_B(c) ((c) == UINT8_C(0x42) || (c) == UINT8_C(0x62))
#define LANG_CHAR_IS_C(c) ((c) == UINT8_C(0x43) || (c) == UINT8_C(0x63))
#define LANG_CHAR_IS_D(c) ((c) == UINT8_C(0x44) || (c) == UINT8_C(0x64))
#define LANG_CHAR_IS_E(c) ((c) == UINT8_C(0x45) || (c) == UINT8_C(0x65))
#define LANG_CHAR_IS_F(c) ((c) == UINT8_C(0x46) || (c) == UINT8_C(0x66))
#define LANG_CHAR_IS_G(c) ((c) == UINT8_C(0x47) || (c) == UINT8_C(0x67))
#define LANG_CHAR_IS_H(c) ((c) == UINT8_C(0x48) || (c) == UINT8_C(0x68))
#define LANG_CHAR_IS_I(c) ((c) == UINT8_C(0x49) || (c) == UINT8_C(0x69))
#define LANG_CHAR_IS_J(c) ((c) == UINT8_C(0x4A) || (c) == UINT8_C(0x6A))
#define LANG_CHAR_IS_K(c) ((c) == UINT8_C(0x4B) || (c) == UINT8_C(0x6B))
#define LANG_CHAR_IS_L(c) ((c) == UINT8_C(0x4C) || (c) == UINT8_C(0x6C))
#define LANG_CHAR_IS_M(c) ((c) == UINT8_C(0x4D) || (c) == UINT8_C(0x6D))
#define LANG_CHAR_IS_N(c) ((c) == UINT8_C(0x4E) || (c) == UINT8_C(0x6E))
#define LANG_CHAR_IS_O(c) ((c) == UINT8_C(0x4F) || (c) == UINT8_C(0x6F))
#define LANG_CHAR_IS_P(c) ((c) == UINT8_C(0x50) || (c) == UINT8_C(0x70))
#define LANG_CHAR_IS_Q(c) ((c) == UINT8_C(0x51) || (c) == UINT8_C(0x71))
#define LANG_CHAR_IS_R(c) ((c) == UINT8_C(0x52) || (c) == UINT8_C(0x72))
#define LANG_CHAR_IS_S(c) ((c) == UINT8_C(0x53) || (c) == UINT8_C(0x73))
#define LANG_CHAR_IS_T(c) ((c) == UINT8_C(0x54) || (c) == UINT8_C(0x74))
#define LANG_CHAR_IS_U(c) ((c) == UINT8_C(0x55) || (c) == UINT8_C(0x75))
#define LANG_CHAR_IS_V(c) ((c) == UINT8_C(0x56) || (c) == UINT8_C(0x76))
#define LANG_CHAR_IS_W(c) ((c) == UINT8_C(0x57) || (c) == UINT8_C(0x77))
#define LANG_CHAR_IS_X(c) ((c) == UINT8_C(0x58) || (c) == UINT8_C(0x78))
#define LANG_CHAR_IS_Y(c) ((c) == UINT8_C(0x59) || (c) == UINT8_C(0x79))
#define LANG_CHAR_IS_Z(c) ((c) == UINT8_C(0x5A) || (c) == UINT8_C(0x7A))
#define LANG_CHAR_IS_DIGIT(c) (UINT8_C(0x30) <= (c) && (c) <= UINT8_C(0x39))
#define LANG_CHAR_IS_SINGLETON(c) (LANG_CHAR_IS_DIGIT(c) || (UINT8_C(0x41) <= (c) && (c) <= UINT8_C(0x57)) || (UINT8_C(0x59) <= (c) && (c) <= UINT8_C(0x5A)) || (UINT8_C(0x61) <= (c) && (c) <= UINT8_C(0x77)) || (UINT8_C(0x79) <= (c) && (c) <= UINT8_C(0x7A)))
#define LANG_CHAR_IS_ALPHA(c) ((UINT8_C(0x41) <= (c) && (c) <= UINT8_C(0x5A)) || (UINT8_C(0x61) <= (c) && (c) <= UINT8_C(0x7A)))
#define LANG_CHAR_IS_ALPHANUM(c) (LANG_CHAR_IS_DIGIT(c) || LANG_CHAR_IS_ALPHA(c))

namespace lang {

using namespace ex;
using namespace ptr;
using namespace std;

#include "lang/lang_arrays.h"

template<typename iter>
bool isLanguageTag(iter begin, iter end);

template<typename iter>
bool isLangTag(iter begin, iter end);

template<typename iter>
bool isPrivateUse(iter begin, iter end);

template<typename iter>
bool isGrandfathered(iter begin, iter end);

template<typename iter>
bool isLanguage(iter begin, iter end);

template<typename iter>
bool isScript(iter begin, iter end);

template<typename iter>
bool isRegion(iter begin, iter end);

template<typename iter>
bool isVariant(iter begin, iter end);

template<typename iter>
bool isExtension(iter begin, iter end);

template<typename iter>
bool isExtLang(iter begin, iter end);

template<typename iter>
bool isIrregular(iter begin, iter end);

template<typename iter>
bool isRegular(iter begin, iter end);

enum LangTagPart {
  LANGUAGE, SCRIPT, REGION, VARIANTS, EXTENSIONS, PRIVATE_USE
};

class LangTag {
protected:
  DPtr<uint8_t> *ascii;
  bool canonical;
  bool extlang_form;
  static const uint8_t *lookup(const uint8_t *key,
                               size_t len,
                               const void **values,
                               size_t &outlen) throw();
  static bool compare_first_only(const uint8_t *a, const uint8_t *b);
  bool replaceSection(DPtr<uint8_t> *part, const void **lookup_array)
      throw(BadAllocException);
  void normalizePart(enum LangTagPart p)
      throw(BadAllocException);
public:
  LangTag() throw(BadAllocException); // "i-default"
  LangTag(DPtr<uint8_t> *ascii)
      throw(SizeUnknownException, MalformedLangTagException);
  LangTag(const LangTag &copy) throw();
  virtual ~LangTag() throw();

  // Final Methods
  DPtr<uint8_t> *getASCIIString() throw();
  DPtr<uint8_t> *getPart(const enum LangTagPart part) const throw();
  bool isPrivateUse() const throw();
  bool isGrandfathered() const throw();
  bool isRegularGrandfathered() const throw();
  bool isIrregularGrandfathered() const throw();

  // Virtual Methods
  virtual LangTag *normalize() throw(BadAllocException);
  virtual LangTag *extlangify() throw(BadAllocException);

  // Operators
  LangTag &operator=(const LangTag &rhs) throw();
  bool operator==(const LangTag &rhs) throw();
  bool operator!=(const LangTag &rhs) throw();
};

}

#include "lang/LangTag-inl.h"

#endif /* __LANG__LANGTAG_H__ */
