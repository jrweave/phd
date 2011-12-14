#ifndef __LANG__LANGRANGE_H__
#define __LANG__LANGRANGE_H__

#include "lang/LangTag.h"
#include "lang/MalformedLangRangeException.h"
#include "ptr/BadAllocException.h"
#include "ptr/DPtr.h"
#include "sys/ints.h"

#define LANG_CHAR_ASTERISK UINT32_C(0x2A)

namespace lang {

using namespace ex;
using namespace ptr;
using namespace std;

template<typename iter>
bool isLanguageRange(iter begin, iter end);

template<typename iter>
bool isExtendedLanguageRange(iter begin, iter end);

class LangRange {
protected:
  DPtr<uint8_t> *ascii;
public:
  LangRange() throw(BadAllocException); // "*"
  LangRange(DPtr<uint8_t> *ascii) throw(MalformedLangRangeException);
  LangRange(const LangRange &copy) throw();
  virtual ~LangRange() throw();

  // Final Methods
  DPtr<uint8_t> *getASCIIString() throw();
  bool isBasic() const throw();
  bool matches(LangTag *lang_tag) const throw();
  bool matches(LangTag *lang_tag, bool basic) const throw();

  // Operators
  LangRange &operator=(const LangRange &rhs) throw();
  bool operator==(const LangRange &rhs) throw();
  bool operator!=(const LangRange &rhs) throw();
};

}

#include "lang/LangRange-inl.h"

#endif /* __LANG__LANGRANGE_H__ */
