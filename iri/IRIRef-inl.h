#include "iri/IRIRef.h"

#include <vector>
#include "ptr/MPtr.h"
#include "sys/ints.h"

#include <iostream>

namespace iri {

using namespace ptr;
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
bool isIRIReference(iter begin, iter end) {
  debug("isIRIReference", begin, end);
  return isIRI(begin, end) || isIRelativeRef(begin, end);
}

template<typename iter>
bool isIRI(iter begin, iter end) {
  debug("isIRI", begin, end);
  iter mark;
  for (mark = begin; mark != end && *mark != IRI_CHAR_COLON; ++mark) {
    // loop does the work
  }
  if (mark == end || !isScheme(begin, mark)) {
    return false;
  }
  ++mark;
  begin = mark;
  for (; mark != end && *mark != IRI_CHAR_QUESTION_MARK
      && *mark != IRI_CHAR_HASH; ++mark) {
    // loop does the work
  }
  if (!isIHierPart(begin, mark)) {
    return false;
  }
  if (mark == end) {
    return true;
  }
  if (*mark == IRI_CHAR_QUESTION_MARK) {
    ++mark;
    begin = mark;
    for (; mark != end && *mark != IRI_CHAR_HASH; ++mark) {
      // loop does the work
    }
    if (!isIQuery(begin, mark)) {
      return false;
    }
    if (mark == end) {
      return true;
    }
  }
  ++mark;
  return isIFragment(mark, end);
}

template<typename iter>
bool isIRelativeRef(iter begin, iter end) {
  debug("isRelativeRef", begin, end);
  iter mark;
  for (mark = begin; mark != end && *mark != IRI_CHAR_QUESTION_MARK
      && *mark != IRI_CHAR_HASH; ++mark) {
    // loop does the work
  }
  if (!isIRelativePart(begin, mark)) {
    return false;
  }
  if (mark == end) {
    return true;
  }
  if (*mark == IRI_CHAR_QUESTION_MARK) {
    ++mark;
    begin = mark;
    for (; mark != end && *mark != IRI_CHAR_HASH; ++mark) {
      // loop does the work
    }
    if (!isIQuery(begin, mark)) {
      return false;
    }
    if (mark == end) {
      return true;
    }
  }
  ++mark;
  return isIFragment(mark, end);
}

template<typename iter>
bool isScheme(iter begin, iter end) {
  debug("isScheme", begin, end);
  if (begin == end || !IRI_CHAR_IS_ALPHA(*begin)) {
    return false;
  }
  for (++begin; begin != end; ++begin) {
    if (!IRI_CHAR_IS_ALPHA(*begin) && !IRI_CHAR_IS_DIGIT(*begin) &&
        *begin != IRI_CHAR_PLUS && *begin != IRI_CHAR_HYPHEN &&
        *begin != IRI_CHAR_PERIOD) {
      return false;
    }
  }
  return true;
}

template<typename iter>
bool isIHierPart(iter begin, iter end) {
  debug("isIHierPart", begin, end);
  iter slash;
  slash = begin;
  if (slash != end && *slash == IRI_CHAR_SLASH) {
    ++slash;
    if (slash != end && *slash == IRI_CHAR_SLASH) {
      ++slash;
      iter auth;
      auth = slash;
      for (++slash; slash != end &&
          *slash != IRI_CHAR_SLASH; ++slash) {
        // loop does the work
      }
      if (isIAuthority(auth, slash) && isIPathAbsEmpty(slash, end)) {
        return true;
      }
    }
  }
  return isIPathAbsolute(begin, end)
      || isIPathRootless(begin, end)
      || isIPathEmpty(begin, end);
}

template<typename iter>
bool isIRelativePart(iter begin, iter end) {
  debug("isIRelativePart", begin, end);
  iter slash;
  slash = begin;
  if (slash != end && *slash == IRI_CHAR_SLASH) {
    ++slash;
    if (slash != end && *slash == IRI_CHAR_SLASH) {
      ++slash;
      iter auth;
      auth = slash;
      for (++slash; slash != end &&
          *slash != IRI_CHAR_SLASH; ++slash) {
        // loop does the work
      }
      if (isIAuthority(auth, slash) && isIPathAbsEmpty(slash, end)) {
        return true;
      }
    }
  }
  return isIPathAbsolute(begin, end)
      || isIPathNoScheme(begin, end)
      || isIPathEmpty(begin, end);
}

template<typename iter>
bool isIAuthority(iter begin, iter end) {
  debug("isIAuthority", begin, end);
  iter mark;
  for (mark = begin; mark != end && *mark != IRI_CHAR_AT; ++mark) {
    // loop does the work
  }
  if (mark == end) {
    mark = begin;
  } else {
      if (!isIUserInfo(begin, mark)) {
        return false;
      }
      ++mark;
      begin = mark;
  }
  if (*begin == IRI_CHAR_LEFT_SQUARE_BRACKET) {
    for (; mark != end && *mark != IRI_CHAR_RIGHT_SQUARE_BRACKET; ++mark) {
      // loop does the work
    }
    if (mark == end) {
      return false;
    }
    ++mark;
    return isIHost(begin, mark) && (mark == end ||
        (*mark == IRI_CHAR_COLON && isPort(++mark, end)));
  }
  for (; mark != end && *mark != IRI_CHAR_COLON; ++mark) {
    // loop does the work
  }
  return isIHost(begin, mark) && (mark == end || isPort(++mark, end));
}

template<typename iter>
bool isIUserInfo(iter begin, iter end) {
  debug("isIUserInfo", begin, end);
  for(; begin != end; ++begin) {
    if (*begin != IRI_CHAR_COLON && !IRIRef::isIUnreserved(*begin) &&
        !IRIRef::isSubDelim(*begin)) {
      iter start = begin;
      if (++begin == end) return false;
      if (++begin == end) return false;
      if (++begin == end) return false;
      if (!isPctEncoded(start, begin)) {
        return false;
      }
    }
  }
  return true;
}

template<typename iter>
bool isIHost(iter begin, iter end) {
  debug("isIHost", begin, end);
  return isIPLiteral(begin, end) || isIPv4Address(begin, end) ||
      isIRegName(begin, end);
}

template<typename iter>
bool isIPLiteral(iter begin, iter end) {
  debug("isIPLiteral", begin, end);
  if (begin == end || *begin != IRI_CHAR_LEFT_SQUARE_BRACKET) {
    return false;
  }
  ++begin;
  if (begin == end) {
    return false;
  }
  iter start, finish;
  start = begin;
  finish = begin;
  for (++begin; begin != end; ++begin) {
    ++finish;
  }
  if (*finish != IRI_CHAR_RIGHT_SQUARE_BRACKET) {
    return false;
  }
  return isIPv6Address(start, finish) || isIPvFutureAddress(start, finish);
}

template<typename iter>
bool isIPv4Address(iter begin, iter end) {
  debug("isIPv4Address", begin, end);
  if (begin == end) {
    return false;
  }
  vector<uint32_t> part;
  bool last = false;
  size_t nparts = 0;
  for (;;) {
    if (nparts > 4) {
      return false;
    }
    if (begin == end || *begin == IRI_CHAR_PERIOD) {
      bool valid = part.size() > 0 && (
        (part.size() == 1 && IRI_CHAR_IS_DIGIT(part[0]))           ||
        (part.size() == 2 && part[0] != IRI_CHAR_ZERO &&
         IRI_CHAR_IS_DIGIT(part[0]) && IRI_CHAR_IS_DIGIT(part[1])) ||
        (part.size() == 3 && part[0] == IRI_CHAR_ONE &&
         IRI_CHAR_IS_DIGIT(part[1]) && IRI_CHAR_IS_DIGIT(part[2])) ||
        (part.size() == 3 && part[0] == IRI_CHAR_TWO &&
         IRI_CHAR_ZERO <= part[1] && part[1] <= IRI_CHAR_FOUR &&
         IRI_CHAR_IS_DIGIT(part[2]))                               ||
        (part.size() == 3 && part[0] == IRI_CHAR_TWO &&
         part[1] == IRI_CHAR_FIVE && IRI_CHAR_ZERO <= part[2] &&
         part[2] <= IRI_CHAR_FIVE)                                 );
      if (!valid) {
        return false;
      } 
      part.clear();
      ++nparts;
      if (last) {
        return nparts == 4;
      }
    } else {
      part.push_back(*begin);
    }
    ++begin;
    if (begin == end) {
      last = true;
    }
  }
}

template<typename iter>
bool isIPv6Address(iter begin, iter end) {
  debug("isIPv6Address", begin, end);
  if (begin == end) {
    return false;
  }
  vector<uint32_t> part;
  iter start = begin;
  bool found_double_colon = false;
  size_t nparts = 0;
  for (;;) {
    if (nparts > 8) {
      return false;
    }
    if (*begin == IRI_CHAR_PERIOD) {
      return (found_double_colon || nparts == 6)
          && isIPv4Address(start, end);
    } else if (*begin == IRI_CHAR_COLON) {
      if (part.empty()) {
        if (found_double_colon) {
          return false;
        }
        if (nparts == 0) {
          if (++begin == end) {
            return false;
          }
          if (*begin != IRI_CHAR_COLON) {
            return false;
          }
        }
        found_double_colon = true;
      } else {
        vector<uint32_t>::iterator it;
        for (it = part.begin(); it != part.end(); ++it) {
          if (!IRI_CHAR_IS_HEXDIG(*it)) {
            return false;
          }
        }
        part.clear();
        ++nparts;
        start = begin;
        ++start;
      }
    } else {
      part.push_back(*begin);
    }
    ++begin;
    if (begin == end) {
      if (!part.empty()) {
        vector<uint32_t>::iterator it;
        for (it = part.begin(); it != part.end(); ++it) {
          if (!IRI_CHAR_IS_HEXDIG(*it)) {
            return false;
          }
        }
        ++nparts;
      }
      return found_double_colon || nparts == 8;
    }
  }
}

template<typename iter>
bool isIPvFutureAddress(iter begin, iter end) {
  debug("isIPvFutureAddress", begin, end);
  // "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
  if (begin == end) {
    return false;
  }
  if (*begin != IRI_CHAR_LOWERCASE_V) {
    return false;
  }
  ++begin;
  if (begin == end || !IRI_CHAR_IS_HEXDIG(*begin)) {
    return false;
  }
  for (++begin; begin != end && IRI_CHAR_IS_HEXDIG(*begin); ++begin) {
    // loop finds the first non hexdig
  }
  if (begin == end || *begin != IRI_CHAR_PERIOD) {
    return false;
  }
  ++begin;
  if (begin == end) {
    return false;
  }
  for (; begin != end; ++begin) {
    if (*begin != IRI_CHAR_COLON &&
        !IRIRef::isUnreserved(*begin) &&
        !IRIRef::isSubDelim(*begin)) {
      return false;
    }
  }
  return true;
}

template<typename iter>
bool isIRegName(iter begin, iter end) {
  debug("isIRegName", begin, end);
  // *( iunreserved / pct-encoded / sub-delims )
  for (; begin != end; ++begin) {
    if (!IRIRef::isIUnreserved(*begin) && !IRIRef::isSubDelim(*begin)) {
      iter start = begin;
      if (++begin == end) return false;
      if (++begin == end) return false;
      if (++begin == end) return false;
      if (!isPctEncoded(start, begin)) {
        return false;
      }
    }
  }
  return true;
}

template<typename iter>
bool isPort(iter begin, iter end) {
  debug("isPort", begin, end);
  for (; begin != end; ++begin) {
    if (!IRI_CHAR_IS_DIGIT(*begin)) {
      return false;
    }
  }
  return true;
}

template<typename iter>
bool isIPath(iter begin, iter end) {
  debug("isIPath", begin, end);
  return isIPathEmpty(begin, end) ||
      isIPathAbsEmpty(begin, end) || isIPathAbsolute(begin, end) ||
      isIPathNoScheme(begin, end) || isIPathRootless(begin, end);
}

template<typename iter>
bool isIPathAbsEmpty(iter begin, iter end) {
  debug("isIPathAbsEmpty", begin, end);
  if (begin == end) {
    return true;
  }
  if (*begin != IRI_CHAR_SLASH) {
    return false;
  }
  vector<uint32_t> part;
  for (++begin; begin != end; ++begin) {
    if (*begin == IRI_CHAR_SLASH) {
      if (!isISegment(part.begin(), part.end())) {
        return false;
      }
      part.clear();
    } else {
      part.push_back(*begin);
    }
  }
  return isISegment(part.begin(), part.end());
}

template<typename iter>
bool isIPathAbsolute(iter begin, iter end) {
  debug("isIPathAbsolute", begin, end);
  if (begin == end || *begin != IRI_CHAR_SLASH) {
    return false;
  }
  ++begin;
  return begin == end || isIPathRootless(begin, end);
}

template<typename iter>
bool isIPathNoScheme(iter begin, iter end) {
  debug("isIPathNoScheme", begin, end);
  // isegment-nz-nc *( "/" isegment )
  iter start = begin;
  for (; begin != end && *begin != IRI_CHAR_SLASH; ++begin) {
    // loop does the work
  }
  return isISegmentNZNC(start, begin) && isIPathAbsEmpty(begin, end);
}

template<typename iter>
bool isIPathRootless(iter begin, iter end) {
  debug("isIPathRootless", begin, end);
  iter start = begin;
  for (; begin != end && *begin != IRI_CHAR_SLASH; ++begin) {
    // loop does the work
  }
  return isISegmentNZ(start, begin) && isIPathAbsEmpty(begin, end);
}

template<typename iter>
bool isIPathEmpty(iter begin, iter end) {
  debug("isIPathEmpty", begin, end);
  return begin == end;
}

template<typename iter>
bool isISegment(iter begin, iter end) {
  debug("isISegment", begin, end);
  for (; begin != end; ++begin) {
    if (!IRIRef::isIPChar(*begin)) {
      return false;
    }
  }
  return true;
}

template<typename iter>
bool isISegmentNZ(iter begin, iter end) {
  debug("isISegmentNZ", begin, end);
  return begin != end && isISegment(begin, end);
}

template<typename iter>
bool isISegmentNZNC(iter begin, iter end) {
  debug("isISegmentNZNC", begin, end);
  // 1*( iunreserved / pct-encoded / sub-delims / "@" )
  if (begin == end) {
      return false;
  }
  for (; begin != end; ++begin) {
    if (*begin != IRI_CHAR_AT && !IRIRef::isIUnreserved(*begin) &&
        !IRIRef::isSubDelim(*begin)) {
      iter start = begin;
      if (++begin == end) return false;
      if (++begin == end) return false;
      if (++begin == end) return false;
      if (!isPctEncoded(start, begin)) {
        return false;
      }
    }
  }
  return true;
}

template<typename iter>
bool isIQuery(iter begin, iter end) {
  debug("isIQuery", begin, end);
  // *( ipchar / iprivate / "/" / "?" )
  for (; begin != end; ++begin) {
    if (*begin != IRI_CHAR_SLASH && *begin != IRI_CHAR_QUESTION_MARK &&
        !IRIRef::isIPChar(*begin) && !IRIRef::isIPrivate(*begin)) {
      return false;
    }
  }
  return true;
}

template<typename iter>
bool isIFragment(iter begin, iter end) {
  debug("isIFragment", begin, end);
  // *( ipchar / "/" / "?" )
  for (; begin != end; ++begin) {
    if (*begin != IRI_CHAR_SLASH && *begin != IRI_CHAR_QUESTION_MARK &&
        !IRIRef::isIPChar(*begin)) {
      return false;
    }
  }
  return true;
}

template<typename iter>
bool isPctEncoded(iter begin, iter end) {
  debug("isPctEncoded", begin, end);
  if (begin == end || *begin != IRI_CHAR_PERCENT) {
    return false;
  }
  ++begin;
  if (begin == end || !IRI_CHAR_IS_HEXDIG(*begin)) {
    return false;
  }
  ++begin;
  return begin != end && IRI_CHAR_IS_HEXDIG(*begin);
}

template<typename iter>
IRIRef *parseIRIRef(iter begin, iter end) throw(BadAllocException) {
  if (!isIRIReference(begin, end)) {
    return NULL;
  }
  size_t len;
  iter temp = begin;
  for (len = 0; temp != end; ++len) {
    ++temp;
  }
  uint8_t *utf8str = (uint8_t *)calloc(len << 2, sizeof(uint8_t));
  if (utf8str == NULL) {
    THROW(BadAllocException, (len << 2) * sizeof(uint8_t));
  }
  len = utf8enc(begin, end, utf8str);
  uint8_t *utf8str2 = (uint8_t *)realloc(utf8str, len*sizeof(uint8_t));
  if (utf8str2 == NULL) {
    free(utf8str);
    THROW(BadAllocException, len*sizeof(uint8_t));
  }
  try {
    return new MPtr<uint8_t>(utf8str2, len);
  } RETHROW_BAD_ALLOC
}

}
