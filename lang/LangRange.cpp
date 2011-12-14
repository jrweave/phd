#include "lang/LangRange.h"

#include "ptr/alloc.h"
#include "ptr/MPtr.h"

namespace lang {

using namespace ex;
using namespace ptr;
using namespace std;

LangRange::LangRange() THROWS(BadAllocException) {
  NEW(this->ascii, MPtr<uint8_t>, 1);
  (*this->ascii)[0] = LANG_CHAR_ASTERISK;
}
TRACE(BadAllocException, "(trace)")

LangRange::LangRange(DPtr<uint8_t> *ascii) throw(MalformedLangRangeException) {
  if (!isExtendedLanguageRange(ascii->dptr(), ascii->dptr() + ascii->size())) {
    THROW(MalformedLangRangeException, ascii);
  }
  this->ascii = ascii;
  this->ascii->hold();
}

LangRange::LangRange(const LangRange &copy) throw() {
  this->ascii = copy.ascii;
  this->ascii->hold();
}

LangRange::~LangRange() throw() {
  this->ascii->drop();
}

DPtr<uint8_t> *LangRange::getASCIIString() throw() {
  this->ascii->hold();
  return this->ascii;
}

bool LangRange::isBasic() const throw() {
  uint8_t *begin = this->ascii->dptr();
  uint8_t *end = begin + this->ascii->size();
  for (; begin != end; ++begin) {
    if (*begin == LANG_CHAR_ASTERISK) {
      return this->ascii->size() == 1;
    }
  }
  return true;
}

bool LangRange::matches(LangTag *lang_tag) const throw() {
  return this->matches(lang_tag, this->isBasic());
}

bool LangRange::matches(LangTag *lang_tag, bool basic) const throw() {
  if (this->ascii->size() == 1 && (*(this->ascii))[0] == LANG_CHAR_ASTERISK) {
    return true;
  }

  DPtr<uint8_t> *tag = lang_tag->getASCIIString();

  if (basic) {
    if (tag->size() < this->ascii->size()) {
      tag->drop();
      return false;
    }
    uint8_t *rbegin = this->ascii->dptr();
    uint8_t *rend = rbegin + this->ascii->size();
    uint8_t *tbegin = tag->dptr();
    uint8_t *tend = tbegin + tag->size();
    for (; rbegin != rend; ++rbegin) {
      if (*rbegin != *tbegin) {
        uint8_t c[2];
        c[0] = *rbegin;
        c[1] = *tbegin;
        if (c[0] >= LANG_CHAR_LOWERCASE_A) {
          c[0] -= LANG_CHAR_LOWERCASE_A - LANG_CHAR_UPPERCASE_A;
        }
        if (c[1] >= LANG_CHAR_LOWERCASE_A) {
          c[1] -= LANG_CHAR_LOWERCASE_A - LANG_CHAR_UPPERCASE_A;
        }
        if (c[0] != c[1]) {
          tag->drop();
          return false;
        }
      }
      ++tbegin;
    }
    bool ret = tbegin == tend || *tbegin == LANG_CHAR_HYPHEN;
    tag->drop();
    return ret;
  }

  uint8_t *rbegin = this->ascii->dptr();
  uint8_t *rmark = rbegin;
  uint8_t *rend = rbegin + this->ascii->size();
  for (; rmark != rend && *rmark != LANG_CHAR_HYPHEN; ++rmark) {
    // loop finds hyphen or end
  }

  uint8_t *tbegin = tag->dptr();
  uint8_t *tmark = tbegin;
  uint8_t *tend = tbegin + tag->size();
  for (; tmark != tend && *tmark != LANG_CHAR_HYPHEN; ++tmark) {
    // loop finds hyphen or end
  }

  if (*rbegin == LANG_CHAR_ASTERISK) {
    rbegin = rmark;
    tbegin = tmark;
  } else {
    if (rmark - rbegin != tmark - tbegin) {
      tag->drop();
      return false;
    }

    for (; rbegin != rmark; ++rbegin) {
      if (*rbegin != *tbegin) {
        uint8_t c[2];
        c[0] = *rbegin;
        c[1] = *tbegin;
        if (c[0] >= LANG_CHAR_LOWERCASE_A) {
          c[0] -= LANG_CHAR_LOWERCASE_A - LANG_CHAR_UPPERCASE_A;
        }
        if (c[1] >= LANG_CHAR_LOWERCASE_A) {
          c[1] -= LANG_CHAR_LOWERCASE_A - LANG_CHAR_UPPERCASE_A;
        }
        if (c[0] != c[1]) {
          tag->drop();
          return false;
        }
      }
      ++tbegin;
    }
  }
  if (rbegin != rend) {
    rbegin = ++rmark;
    for (; rmark != rend && *rmark != LANG_CHAR_HYPHEN; ++rmark) {
      // loop finds hyphen or end
    }
  }
  if (tbegin != tend) {
    tbegin = ++tmark;
    for (; tmark != tend && *tmark != LANG_CHAR_HYPHEN; ++tmark) {
      // loop finds hyphen or end
    }
  }
  while (rbegin != rend) {
    if (*rbegin == LANG_CHAR_ASTERISK) {
      if (rmark == rend) {
        rbegin = rend;
      } else {
        rbegin = ++rmark;
        for (; rmark != rend && *rmark != LANG_CHAR_HYPHEN; ++rmark) {
          // loop finds hyphen or end
        }
      }
      continue;
    }
    if (tbegin == tend) {
      tag->drop();
      return false;
    }
    if (rmark - rbegin == tmark - tbegin) {
      for (; rbegin != rmark; ++rbegin) {
        if (*rbegin != *tbegin) {
          uint8_t c[2];
          c[0] = *rbegin;
          c[1] = *tbegin;
          if (c[0] >= LANG_CHAR_LOWERCASE_A) {
            c[0] -= LANG_CHAR_LOWERCASE_A - LANG_CHAR_UPPERCASE_A;
          }
          if (c[1] >= LANG_CHAR_LOWERCASE_A) {
            c[1] -= LANG_CHAR_LOWERCASE_A - LANG_CHAR_UPPERCASE_A;
          }
          if (c[0] != c[1]) {
            break;
          }
        }
        ++tbegin;
      }
      if (rmark == rend) {
        rbegin = rend;
      } else {
        rbegin = ++rmark;
        for (; rmark != rend && *rmark != LANG_CHAR_HYPHEN; ++rmark) {
          // loop finds hyphen or end
        }
      }
      if (tmark == tend) {
        tbegin = tend;
      } else {
        tbegin = ++tmark;
        for (; tmark != tend && *tmark != LANG_CHAR_HYPHEN; ++tmark) {
          // loop finds hyphen or end
        }
      }
      continue;
    }
    if (tmark - tbegin == 1) {
      tag->drop();
      return false;
    }
    if (tmark == tend) {
      tbegin = tend;
    } else {
      tbegin = ++tmark;
      for (; tmark != tend && *tmark != LANG_CHAR_HYPHEN; ++tmark) {
        // loop finds hyphen or end
      }
    }
  }
  tag->drop();
  return true;
}

LangRange &LangRange::operator=(const LangRange &rhs) throw() {
  this->ascii->drop();
  this->ascii = rhs.ascii;
  this->ascii->hold();
  return *this;
}

bool LangRange::operator==(const LangRange &rhs) throw() {
  if (this->ascii->size() != rhs.ascii->size()) {
    return false;
  }
  size_t i;
  for (i = 0; i < this->ascii->size(); ++i) {
    if ((*(this->ascii))[i] != (*(rhs.ascii))[i]) {
      uint8_t c[2];
      c[0] = (*(this->ascii))[i];
      c[1] = (*(rhs.ascii))[i];
      if (c[0] >= LANG_CHAR_LOWERCASE_A) {
        c[0] -= LANG_CHAR_LOWERCASE_A - LANG_CHAR_UPPERCASE_A;
      }
      if (c[1] >= LANG_CHAR_LOWERCASE_A) {
        c[1] -= LANG_CHAR_LOWERCASE_A - LANG_CHAR_UPPERCASE_A;
      }
      if (c[0] != c[1]) {
        return false;
      }
    }
  }
  return true;
}

bool LangRange::operator!=(const LangRange &rhs) throw() {
  return !(*this == rhs);
}

}
