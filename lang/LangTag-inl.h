#include "lang/LangTag.h"

#include <iostream>

namespace lang {

using namespace std;

#if 0
template<typename iter>
void debug(const char *label, iter begin, iter end) {
  cerr << label << " ? ";
  for (; begin != end; ++begin) {
    cerr << (char)*begin;
  }
  cerr << endl;
}
#else
#define debug(a, b, c)
#endif

template<typename iter>
bool isLanguageTag(iter begin, iter end) {
  return isLangTag(begin, end)
      || isPrivateUse(begin, end)
      || isGrandfathered(begin, end);
}

template<typename iter>
bool isLangTag(iter begin, iter end) {

  iter m1, m2;
  for (m1 = begin; m1 != end && *m1 != LANG_CHAR_HYPHEN; ++m1) {
    // loop does the work
  }

  // language
  if (!isLanguage(begin, m1)) {
    return false;
  }
  m2 = m1;
  do {
    if (m2 == end) {
      return true;
    }
    m1 = m2;
    for (++m2; m2 != end && *m2 != LANG_CHAR_HYPHEN; ++m2) {
      // loop does the work
    }
  } while (isLanguage(begin, m2));
  if (++m1 == end) {
    return false;
  }
  begin = m1;
  for (; m1 != end && *m1 != LANG_CHAR_HYPHEN; ++m1) {
    // loop does the work
  }
  
  // script
  if (isScript(begin, m1)) {
    if (m1 == end) {
      return true;
    }
    if (++m1 == end) {
      return false;
    }
    begin = m1;
    for (; m1 != end && *m1 != LANG_CHAR_HYPHEN; ++m1) {
      // loop does the work
    }
  }

  // region
  if (isRegion(begin, m1)) {
    if (m1 == end) {
      return true;
    }
    if (++m1 == end) {
      return false;
    }
    begin = m1;
    for (; m1 != end && *m1 != LANG_CHAR_HYPHEN; ++m1) {
      // loop does the work
    }
  }

  // variants
  while (isVariant(begin, m1)) {
    if (m1 == end) {
      return true;
    }
    if (++m1 == end) {
      return false;
    }
    begin = m1;
    for (; m1 != end && *m1 != LANG_CHAR_HYPHEN; ++m1) {
      // loop does the work
    }
  }

  // extensions
  m2 = begin;
  ++m2;
  if (m1 == m2) {
    while (!LANG_CHAR_IS_X(*begin)) {
      for (++m1; m1 != end && *m1 != LANG_CHAR_HYPHEN; ++m1) {
        // loop does the work
      }
      if (!isExtension(begin, m1)) {
        return false;
      }
      m2 = m1;
      do {
        if (m2 == end) {
          return true;
        }
        m1 = m2;
        for (++m2; m2 != end && *m2 != LANG_CHAR_HYPHEN; ++m2) {
          // loop does the work
        }
      } while (isExtension(begin, m2));
      if (++m1 == end) {
        return false;
      }
      begin = m1;
      if (++m1 == end || *m1 != LANG_CHAR_HYPHEN) {
        return false;
      }
    }
  }

  // privateuse
  return isPrivateUse(begin, end);
}

template<typename iter>
bool isLanguage(iter begin, iter end) {
  iter mark;
  size_t i = 0;
  for (mark = begin; mark != end && *mark != LANG_CHAR_HYPHEN; ++mark) {
    if (++i > 8) {
      return false;
    }
    if (!LANG_CHAR_IS_ALPHA(*mark)) {
      return false;
    }
  }
  if (i < 2) {
    return false;
  }
  return mark == end || (i <= 3 && isExtLang(++mark, end));
}

template<typename iter>
bool isExtLang(iter begin, iter end) {
  size_t n;
  for (n = 0; n < 3 && begin != end; ++n) {
    if (!(LANG_CHAR_IS_ALPHA(*begin)
          && ++begin != end && LANG_CHAR_IS_ALPHA(*begin)
          && ++begin != end && LANG_CHAR_IS_ALPHA(*begin)
          && (++begin == end
              || (*begin == LANG_CHAR_HYPHEN && ++begin != end)))) {
      return false;
    }
  }
  return 1 <= n && n <= 3;
}

template<typename iter>
bool isScript(iter begin, iter end) {
  return begin != end && LANG_CHAR_IS_ALPHA(*begin)
      && ++begin != end && LANG_CHAR_IS_ALPHA(*begin)
      && ++begin != end && LANG_CHAR_IS_ALPHA(*begin)
      && ++begin != end && LANG_CHAR_IS_ALPHA(*begin)
      && ++begin == end;
}

template<typename iter>
bool isRegion(iter begin, iter end) {
  if (begin == end) {
    return false;
  }
  if (LANG_CHAR_IS_ALPHA(*begin)) {
    return ++begin != end && LANG_CHAR_IS_ALPHA(*begin)
        && ++begin == end;
  }
  return LANG_CHAR_IS_DIGIT(*begin)
      && ++begin != end && LANG_CHAR_IS_DIGIT(*begin)
      && ++begin != end && LANG_CHAR_IS_DIGIT(*begin)
      && ++begin == end;
}

template<typename iter>
bool isVariant(iter begin, iter end) {
  if (begin == end) {
    return false;
  }
  iter mark = begin;
  size_t i;
  for (i = 0; i < 8 && mark != end; ++i) {
    if (!LANG_CHAR_IS_ALPHANUM(*mark)) {
      return false;
    }
    ++mark;
  }
  return (i == 4 && LANG_CHAR_IS_DIGIT(*begin))
      || (i >= 5 && mark == end);
}

template<typename iter>
bool isExtension(iter begin, iter end) {
  if (!(begin != end && LANG_CHAR_IS_SINGLETON(*begin)
        && ++begin != end && *begin == LANG_CHAR_HYPHEN
        && ++begin != end)) {
    return false;
  }
  while (begin != end) {
    size_t i = 0;
    for (; begin != end && *begin != LANG_CHAR_HYPHEN; ++begin) {
      if (!LANG_CHAR_IS_ALPHANUM(*begin)) {
        return false;
      }
      ++i;
    }
    if (i < 2 || i > 8 || (begin != end && ++begin == end)) {
      return false;
    }
  }
  return true;
}

template<typename iter>
bool isPrivateUse(iter begin, iter end) {
  if (!(begin != end && LANG_CHAR_IS_X(*begin)
        && ++begin != end && *begin == LANG_CHAR_HYPHEN
        && ++begin != end)) {
    return false;
  }
  while (begin != end) {
    size_t i = 0;
    for (; begin != end && *begin != LANG_CHAR_HYPHEN; ++begin) {
      if (!LANG_CHAR_IS_ALPHANUM(*begin)) {
        return false;
      }
      ++i;
    }
    if (i < 1 || i > 8 || (begin != end && ++begin == end)) {
      return false;
    }
  }
  return true;
}

template<typename iter>
bool isGrandfathered(iter begin, iter end) {
  return isIrregular(begin, end) || isRegular(begin, end);
}

template<typename iter>
bool isIrregular(iter begin, iter end) {
  if (begin == end) {
    return false;
  }
  if (LANG_CHAR_IS_E(*begin)) {
    return ++begin != end && LANG_CHAR_IS_N(*begin)
        && ++begin != end && *begin == LANG_CHAR_HYPHEN
        && ++begin != end && LANG_CHAR_IS_G(*begin)
        && ++begin != end && LANG_CHAR_IS_B(*begin)
        && ++begin != end && *begin == LANG_CHAR_HYPHEN
        && ++begin != end && LANG_CHAR_IS_O(*begin)
        && ++begin != end && LANG_CHAR_IS_E(*begin)
        && ++begin != end && LANG_CHAR_IS_D(*begin)
        && ++begin == end;
  }
  if (LANG_CHAR_IS_I(*begin)) {
    if (++begin == end || *begin != LANG_CHAR_HYPHEN || ++begin == end) {
      return false;
    }
    if (LANG_CHAR_IS_A(*begin)) {
      return ++begin != end && LANG_CHAR_IS_M(*begin)
          && ++begin != end && LANG_CHAR_IS_I(*begin)
          && ++begin == end;
    }
    if (LANG_CHAR_IS_B(*begin)) {
      return ++begin != end && LANG_CHAR_IS_N(*begin)
          && ++begin != end && LANG_CHAR_IS_N(*begin)
          && ++begin == end;
    }
    if (LANG_CHAR_IS_D(*begin)) {
      return ++begin != end && LANG_CHAR_IS_E(*begin)
          && ++begin != end && LANG_CHAR_IS_F(*begin)
          && ++begin != end && LANG_CHAR_IS_A(*begin)
          && ++begin != end && LANG_CHAR_IS_U(*begin)
          && ++begin != end && LANG_CHAR_IS_L(*begin)
          && ++begin != end && LANG_CHAR_IS_T(*begin)
          && ++begin == end;
    }
    if (LANG_CHAR_IS_E(*begin)) {
      return ++begin != end && LANG_CHAR_IS_N(*begin)
          && ++begin != end && LANG_CHAR_IS_O(*begin)
          && ++begin != end && LANG_CHAR_IS_C(*begin)
          && ++begin != end && LANG_CHAR_IS_H(*begin)
          && ++begin != end && LANG_CHAR_IS_I(*begin)
          && ++begin != end && LANG_CHAR_IS_A(*begin)
          && ++begin != end && LANG_CHAR_IS_N(*begin)
          && ++begin == end;
    }
    if (LANG_CHAR_IS_H(*begin)) {
      return ++begin != end && LANG_CHAR_IS_A(*begin)
          && ++begin != end && LANG_CHAR_IS_K(*begin)
          && ++begin == end;
    }
    if (LANG_CHAR_IS_K(*begin)) {
      return ++begin != end && LANG_CHAR_IS_L(*begin)
          && ++begin != end && LANG_CHAR_IS_I(*begin)
          && ++begin != end && LANG_CHAR_IS_N(*begin)
          && ++begin != end && LANG_CHAR_IS_G(*begin)
          && ++begin != end && LANG_CHAR_IS_O(*begin)
          && ++begin != end && LANG_CHAR_IS_N(*begin)
          && ++begin == end;
    }
    if (LANG_CHAR_IS_L(*begin)) {
      return ++begin != end && LANG_CHAR_IS_U(*begin)
          && ++begin != end && LANG_CHAR_IS_X(*begin)
          && ++begin == end;
    }
    if (LANG_CHAR_IS_M(*begin)) {
      return ++begin != end && LANG_CHAR_IS_I(*begin)
          && ++begin != end && LANG_CHAR_IS_N(*begin)
          && ++begin != end && LANG_CHAR_IS_G(*begin)
          && ++begin != end && LANG_CHAR_IS_O(*begin)
          && ++begin == end;
    }
    if (LANG_CHAR_IS_N(*begin)) {
      return ++begin != end && LANG_CHAR_IS_A(*begin)
          && ++begin != end && LANG_CHAR_IS_V(*begin)
          && ++begin != end && LANG_CHAR_IS_A(*begin)
          && ++begin != end && LANG_CHAR_IS_J(*begin)
          && ++begin != end && LANG_CHAR_IS_O(*begin)
          && ++begin == end;
    }
    if (LANG_CHAR_IS_P(*begin)) {
      return ++begin != end && LANG_CHAR_IS_W(*begin)
          && ++begin != end && LANG_CHAR_IS_N(*begin)
          && ++begin == end;
    }
    if (LANG_CHAR_IS_T(*begin)) {
      if (++begin == end) {
        return false;
      }
      if (LANG_CHAR_IS_A(*begin)) {
        if (++begin == end) {
          return false;
        }
        if (LANG_CHAR_IS_O(*begin)) {
          return ++begin == end;
        }
        if (LANG_CHAR_IS_Y(*begin)) {
          return ++begin == end;
        }
        return false;
      }
      if (LANG_CHAR_IS_S(*begin)) {
        return ++begin != end && LANG_CHAR_IS_U(*begin)
            && ++begin == end;
      }
      return false;
    }
  }
  if (LANG_CHAR_IS_S(*begin)) {
    if (++begin == end || !LANG_CHAR_IS_G(*begin)
        || ++begin == end || !LANG_CHAR_IS_N(*begin)
        || ++begin == end || *begin != LANG_CHAR_HYPHEN
        || ++begin == end) {
      return false;
    }
    if (LANG_CHAR_IS_B(*begin)) {
      if (++begin == end || !LANG_CHAR_IS_E(*begin)
          || ++begin == end || *begin != LANG_CHAR_HYPHEN
          || ++begin == end) {
        return false;
      }
      if (LANG_CHAR_IS_F(*begin)) {
        return ++begin != end && LANG_CHAR_IS_R(*begin)
            && ++begin == end;
      }
      if (LANG_CHAR_IS_N(*begin)) {
        return ++begin != end && LANG_CHAR_IS_L(*begin)
            && ++begin == end;
      }
      return false;
    }
    if (LANG_CHAR_IS_C(*begin)) {
      return ++begin != end && LANG_CHAR_IS_H(*begin)
          && ++begin != end && *begin == LANG_CHAR_HYPHEN
          && ++begin != end && LANG_CHAR_IS_D(*begin)
          && ++begin != end && LANG_CHAR_IS_E(*begin)
          && ++begin == end;
    }
    return false;
  }
  return false;
}

template<typename iter>
bool isRegular(iter begin, iter end) {
  if (begin == end) {
    return false;
  }
  if (LANG_CHAR_IS_A(*begin)) {
    return ++begin != end && LANG_CHAR_IS_R(*begin)
        && ++begin != end && LANG_CHAR_IS_T(*begin)
        && ++begin != end && *begin == LANG_CHAR_HYPHEN
        && ++begin != end && LANG_CHAR_IS_L(*begin)
        && ++begin != end && LANG_CHAR_IS_O(*begin)
        && ++begin != end && LANG_CHAR_IS_J(*begin)
        && ++begin != end && LANG_CHAR_IS_B(*begin)
        && ++begin != end && LANG_CHAR_IS_A(*begin)
        && ++begin != end && LANG_CHAR_IS_N(*begin)
        && ++begin == end;
  }
  if (LANG_CHAR_IS_C(*begin)) {
    return ++begin != end && LANG_CHAR_IS_E(*begin)
        && ++begin != end && LANG_CHAR_IS_L(*begin)
        && ++begin != end && *begin == LANG_CHAR_HYPHEN
        && ++begin != end && LANG_CHAR_IS_G(*begin)
        && ++begin != end && LANG_CHAR_IS_A(*begin)
        && ++begin != end && LANG_CHAR_IS_U(*begin)
        && ++begin != end && LANG_CHAR_IS_L(*begin)
        && ++begin != end && LANG_CHAR_IS_I(*begin)
        && ++begin != end && LANG_CHAR_IS_S(*begin)
        && ++begin != end && LANG_CHAR_IS_H(*begin)
        && ++begin == end;
  }
  if (LANG_CHAR_IS_N(*begin)) {
    if (++begin == end || !LANG_CHAR_IS_O(*begin)
        || ++begin == end || *begin != LANG_CHAR_HYPHEN
        || ++begin == end) {
      return false;
    }
    if (LANG_CHAR_IS_B(*begin)) {
      return ++begin != end && LANG_CHAR_IS_O(*begin)
          && ++begin != end && LANG_CHAR_IS_K(*begin)
          && ++begin == end;
    }
    if (LANG_CHAR_IS_N(*begin)) {
      return ++begin != end && LANG_CHAR_IS_Y(*begin)
          && ++begin != end && LANG_CHAR_IS_N(*begin)
          && ++begin == end;
    }
    return false;
  }
  if (LANG_CHAR_IS_Z(*begin)) {
    if (++begin == end || !LANG_CHAR_IS_H(*begin)
        || ++begin == end || *begin != LANG_CHAR_HYPHEN
        || ++begin == end) {
      return false;
    }
    if (LANG_CHAR_IS_G(*begin)) {
      return ++begin != end && LANG_CHAR_IS_U(*begin)
          && ++begin != end && LANG_CHAR_IS_O(*begin)
          && ++begin != end && LANG_CHAR_IS_Y(*begin)
          && ++begin != end && LANG_CHAR_IS_U(*begin)
          && ++begin == end;
    }
    if (LANG_CHAR_IS_H(*begin)) {
      return ++begin != end && LANG_CHAR_IS_A(*begin)
          && ++begin != end && LANG_CHAR_IS_K(*begin)
          && ++begin != end && LANG_CHAR_IS_K(*begin)
          && ++begin != end && LANG_CHAR_IS_A(*begin)
          && ++begin == end;
    }
    if (LANG_CHAR_IS_M(*begin)) {
      if (++begin == end || !LANG_CHAR_IS_I(*begin)
          || ++begin == end || !LANG_CHAR_IS_N(*begin)) {
        return false;
      }
      if (++begin == end) {
        return true;
      }
      return *begin == LANG_CHAR_HYPHEN
          && ++begin != end && LANG_CHAR_IS_N(*begin)
          && ++begin != end && LANG_CHAR_IS_A(*begin)
          && ++begin != end && LANG_CHAR_IS_N(*begin)
          && ++begin == end;
    }
    if (LANG_CHAR_IS_X(*begin)) {
      return ++begin != end && LANG_CHAR_IS_I(*begin)
          && ++begin != end && LANG_CHAR_IS_A(*begin)
          && ++begin != end && LANG_CHAR_IS_N(*begin)
          && ++begin != end && LANG_CHAR_IS_G(*begin)
          && ++begin == end;
    }
    return false;
  }
  return false;
}

}
