#include "lang/LangRange.h"

namespace lang {

using namespace std;

template<typename iter>
bool isLanguageRange(iter begin, iter end) {
  if (begin == end) {
    return false;
  }
  if (*begin == to_ascii('*')) {
    return ++begin == end;
  }
  size_t i;
  for (i = 0; i < 8 && begin != end && *begin != to_ascii('-'); ++i) {
    if (!is_alpha(*begin)) {
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
  if (*begin != to_ascii('-') || ++begin == end) {
    return false;
  }
  for (;;) {
    for (i = 0; i < 8 && begin != end && *begin != to_ascii('-'); ++i) {
      if (!is_alnum(*begin)) {
        return false;
      }
    }
    if (i < 1) {
      return false;
    }
    if (begin == end) {
      return true;
    }
    if (*begin != to_ascii('-') || ++begin == end) {
      return false;
    }
  }
}

template<typename iter>
bool isExtendedLanguageRange(iter begin, iter end) {
  if (begin == end) {
    return false;
  }
  if (*begin == to_ascii('*')) {
    ++begin;
  } else {
    size_t i = 0;
    for (; i < 8 && begin != end && *begin != to_ascii('-'); ++i) {
      if (!is_alpha(*begin)) {
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
  if (*begin != to_ascii('-') || ++begin == end) {
    return false;
  }
  for (;;) {
    if (*begin == to_ascii('*')) {
      ++begin;
    } else {
      size_t i;
      for (i = 0; i < 8 && begin != end && *begin != to_ascii('-'); ++i) {
        if (!is_alnum(*begin)) {
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
    if (*begin != to_ascii('-') || ++begin == end) {
      return false;
    }
  }
}

}
