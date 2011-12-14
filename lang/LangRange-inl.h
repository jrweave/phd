#include "lang/LangRange.h"

namespace lang {

using namespace std;

template<typename iter>
bool isLanguageRange(iter begin, iter end) {
  if (begin == end) {
    return false;
  }
  if (*begin == LANG_CHAR_ASTERISK) {
    return ++begin == end;
  }
  size_t i;
  for (i = 0; i < 8 && begin != end && *begin != LANG_CHAR_HYPHEN; ++i) {
    if (!LANG_CHAR_IS_ALPHA(*begin)) {
      return false;
    }
    ++begin;
  }
  if (i < 1) {
    return false;
  }
  if (begin == end) {
    return true;
  }
  if (*begin != LANG_CHAR_HYPHEN || ++begin == end) {
    return false;
  }
  for (;;) {
    for (i = 0; i < 8 && begin != end && *begin != LANG_CHAR_HYPHEN; ++i) {
      if (!LANG_CHAR_IS_ALPHANUM(*begin)) {
        return false;
      }
    }
    if (i < 1) {
      return false;
    }
    if (begin == end) {
      return true;
    }
    if (*begin != LANG_CHAR_HYPHEN || ++begin == end) {
      return false;
    }
  }
}

template<typename iter>
bool isExtendedLanguageRange(iter begin, iter end) {
  if (begin == end) {
    return false;
  }
  if (*begin == LANG_CHAR_ASTERISK) {
    ++begin;
  } else {
    size_t i = 0;
    for (; i < 8 && begin != end && *begin != LANG_CHAR_HYPHEN; ++i) {
      if (!LANG_CHAR_IS_ALPHA(*begin)) {
        return false;
      }
      ++begin;
    }
    if (i < 1) {
      return false;
    }
  }
  if (begin == end) {
    return true;
  }
  if (*begin != LANG_CHAR_HYPHEN || ++begin == end) {
    return false;
  }
  for (;;) {
    if (*begin == LANG_CHAR_ASTERISK) {
      ++begin;
    } else {
      size_t i;
      for (i = 0; i < 8 && begin != end && *begin != LANG_CHAR_HYPHEN; ++i) {
        if (!LANG_CHAR_IS_ALPHANUM(*begin)) {
          return false;
        }
        ++begin;
      }
      if (i < 1) {
        return false;
      }
    }
    if (begin == end) {
      return true;
    }
    if (*begin != LANG_CHAR_HYPHEN || ++begin == end) {
      return false;
    }
  }
}

}
