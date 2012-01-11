#include "lang/LangTag.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>
#include "ptr/MPtr.h"
#include "ptr/SizeUnknownException.h"
#include "sys/char.h"

namespace lang {

using namespace ptr;
using namespace std;

#include "lang/lang_arrays.cpp"

LangTag::LangTag() throw(BadAllocException)
    : ascii(NULL), canonical(false), extlang_form(false) {
  try {
    NEW(this->ascii, MPtr<uint8_t>, 9);
  } RETHROW_BAD_ALLOC
  ascii_strcpy(this->ascii->dptr(), "i-default");
}

LangTag::LangTag(DPtr<uint8_t> *ascii)
    throw(SizeUnknownException, MalformedLangTagException)
    : ascii(NULL), canonical(false), extlang_form(false) {
  if (!ascii->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  if (!isLanguageTag(ascii->dptr(), ascii->dptr() + ascii->size())) {
    THROW(MalformedLangTagException, ascii);
  }
  this->ascii = ascii;
  this->ascii->hold();
}

DPtr<uint8_t> *LangTag::getPart(const enum LangTagPart part) const throw() {
  if (this->isGrandfathered()) {
    return NULL;
  }
  if (this->isPrivateUse()) {
    if (part == PRIVATE_USE) {
      this->ascii->hold();
      return this->ascii;
    }
    return NULL;
  }
  size_t offset = 0;
  size_t mark = 0;

  // LANGUAGE
  for (; mark < this->ascii->size()
         && (*(this->ascii))[mark] != to_ascii('-'); ++mark) {
    // loop does the work
  }
  size_t next = mark;
  do {
    if (next == this->ascii->size()) {
      if (part == LANGUAGE) {
        return this->ascii->sub(offset, next);
      }
      return NULL;
    }
    mark = next;
    for (++next; next < this->ascii->size()
                 && (*(this->ascii))[next] != to_ascii('-'); ++next) {
      // loop does the work
    }
  } while (isLanguage(this->ascii->dptr(),
                      this->ascii->dptr() + next));
  if (part == LANGUAGE) {
    return this->ascii->sub(offset, mark - offset);
  }
  offset = mark + 1;
  mark = next;

  // SCRIPT
  if (isScript(this->ascii->dptr() + offset,
               this->ascii->dptr() + mark)) {
    if (part == SCRIPT) {
      return this->ascii->sub(offset, mark - offset);
    }
    if (mark == this->ascii->size()) {
      return NULL;
    }
    offset = ++mark;
    for (; mark < this->ascii->size()
           && (*(this->ascii))[mark] != to_ascii('-'); ++mark) {
      // loop does the work
    }
  } else {
    if (part == SCRIPT) {
      return NULL;
    }
  }

  // REGION
  if (isRegion(this->ascii->dptr() + offset,
               this->ascii->dptr() + mark)) {
    if (part == REGION) {
      return this->ascii->sub(offset, mark - offset);
    }
    if (mark == this->ascii->size()) {
      return NULL;
    }
    offset = ++mark;
    for (; mark < this->ascii->size()
           && (*(this->ascii))[mark] != to_ascii('-'); ++mark) {
      // loop does the work
    }
  } else {
    if (part == REGION) {
      return NULL;
    }
  }

  // VARIANTS
  if (isVariant(this->ascii->dptr() + offset,
                this->ascii->dptr() + mark)) {
    next = mark;
    do {
      if (next == this->ascii->size()) {
        if (part == VARIANTS) {
          return this->ascii->sub(offset, next - offset);
        }
        return NULL;
      }
      mark = next;
      for (++next; next < this->ascii->size()
                   && (*(this->ascii))[next] != to_ascii('-'); ++next) {
        // loop does the work
      }
    } while (isVariant(this->ascii->dptr() + mark + 1,
                       this->ascii->dptr() + next));
    if (part == VARIANTS) {
      return this->ascii->sub(offset, mark - offset);
    }
    offset = mark + 1;
    mark = next;
  } else {
    if (part == VARIANTS) {
      return NULL;
    }
  }

  // EXTENSIONS
  if (to_lower((*(this->ascii))[offset]) == to_ascii('x')) {
    if (part == EXTENSIONS) {
      return NULL;
    }
  } else {
    next = mark;
    do {
      if (next == this->ascii->size()) {
        if (part == EXTENSIONS) {
          return this->ascii->sub(offset, next - offset);
        }
        return NULL;
      }
      mark = next;
      for (++next; next < this->ascii->size()
             && (*(this->ascii))[next] != to_ascii('-'); ++next) {
        // loop does the work
      }
    } while (next - mark > 2 ||
             to_lower((*(this->ascii))[mark + 1]) != to_ascii('x'));
    if (part == EXTENSIONS) {
      return this->ascii->sub(offset, mark - offset);
    }
    offset = ++mark;
  }

  // PRIVATE_USE
  return this->ascii->sub(offset, this->ascii->size() - offset);
}

LangTag *LangTag::normalize() THROWS(BadAllocException) {
  if (this->canonical) {
    return this;
  }

  if (this->ascii->standable()) {
    this->ascii = this->ascii->stand();
  } else {
    DPtr<uint8_t> *a;
    NEW(a, MPtr<uint8_t>, this->ascii->size());
    memcpy(a->dptr(), this->ascii->dptr(),
           this->ascii->size() * sizeof(uint8_t));
    this->ascii->drop();
    this->ascii = a;
  }

  if (this->isGrandfathered()) {
    // just go to lowercase
    uint8_t *begin = this->ascii->dptr();
    uint8_t *end = begin + this->ascii->size();
    for (; begin != end; ++begin) {
      *begin = to_lower(*begin);
    }
    if (this->isIrregularGrandfathered()) {
      // check for regions and uppercase them
      begin = this->ascii->dptr();
      if (to_lower(*begin) == to_ascii('e')) {
        begin[3] = to_upper(begin[3]);
        begin[4] = to_upper(begin[4]);
      } else if (to_lower(*begin) == to_ascii('s')) {
        begin[4] = to_upper(begin[4]);
        begin[5] = to_upper(begin[5]);
        begin[7] = to_upper(begin[7]);
        begin[8] = to_upper(begin[8]);
      }
    }
  }


  DPtr<uint8_t> *part = NULL;

  // Sort extensions by singletons.
  part = this->getPart(EXTENSIONS);
  if (part != NULL) {
    vector<uint8_t *> extmarks;
    uint8_t *mark = part->dptr();
    uint8_t *end = mark + part->size();
    extmarks.push_back(mark);
    while (mark != end) {
      for (++mark; mark != end && *mark != to_ascii('-'); ++mark) {
        // loop does the work
      }
      if (mark != end) {
        ++mark;
        if (mark[1] == to_ascii('-')) {
          extmarks.push_back(mark);
        }
      }
    }
    if (extmarks.size() > 1) {
      map<void *, size_t> lengths;
      vector<uint8_t *>::iterator it;
      for (it = extmarks.begin(); it != extmarks.end(); ++it) {
        if (it + 1 == extmarks.end()) {
          lengths[(void*) *it] = end - *it;
        } else {
          lengths[(void*) *it] = ((*(it + 1)) - *it) - 1;
        }
      }
      stable_sort(extmarks.begin(), extmarks.end(), compare_first_only);
      uint8_t *newexts;
      if (!alloc(newexts, part->size())) {
        THROW(BadAllocException, part->size() * sizeof(uint8_t));
      }
      mark = newexts;
      for (it = extmarks.begin(); it != extmarks.end(); ++it) {
        size_t l = lengths[(void*) *it];
        memcpy(mark, *it, l * sizeof(uint8_t));
        mark += l;
        if (it + 1 != extmarks.end()) {
          *mark = to_ascii('-');
          ++mark;
        }
      }
      memcpy(part->dptr(), newexts, part->size() * sizeof(uint8_t));
      dalloc(newexts);
    }
    part->drop();
  }

  // Replace redundant and grandfathered tags with preferred values
  if (this->replaceSection(this->ascii,
      LANG_REDUNDANT_TAG_PREFERRED_VALUE_VALUES)) {
    return this;
  }
  if (this->replaceSection(this->ascii,
      LANG_GRANDFATHERED_TAG_PREFERRED_VALUE_VALUES)) {
    return this;
  }

  // Replace subtags with preferred values; otherwise, case normalize
  this->normalizePart(LANGUAGE);
  this->normalizePart(SCRIPT);
  this->normalizePart(REGION);
  this->normalizePart(VARIANTS);
  this->normalizePart(EXTENSIONS);
  this->normalizePart(PRIVATE_USE);

  this->extlang_form = false;
  this->canonical = true;
  
  return this;
}
TRACE(BadAllocException, "(trace)")

LangTag *LangTag::extlangify() THROWS(BadAllocException) {
  if (this->extlang_form) {
    return this;
  }
  this->normalize();
  if (this->isGrandfathered()) {
    this->extlang_form = true;
    return this;
  }
  DPtr<uint8_t> *lang = this->getPart(LANGUAGE);
  size_t preflen;
  const uint8_t *prefix = lookup(lang->dptr(), lang->size(),
      LANG_EXTLANG_SUBTAG_PREFIX_VALUES, preflen);
  if (prefix != NULL) {
    DPtr<uint8_t> *s;
    NEW(s, MPtr<uint8_t>, preflen + 1 + this->ascii->size());
    memcpy(s->dptr(), prefix, preflen * sizeof(uint8_t));
    (*s)[preflen] = to_ascii('-');
    memcpy(s->dptr() + preflen + 1, this->ascii->dptr(),
        this->ascii->size() * sizeof(uint8_t));
    this->ascii->drop();
    this->ascii = s;
  }
  lang->drop();
  this->extlang_form = true;
  return this;
}
TRACE(BadAllocException, "(trace)")

LangTag &LangTag::operator=(const LangTag &rhs) throw() {
  this->ascii->drop();
  this->ascii = rhs.ascii;
  this->ascii->hold();
  this->canonical = rhs.canonical;
  this->extlang_form = rhs.extlang_form;
  return *this;
}

bool LangTag::operator==(const LangTag &rhs) throw() {
  if (this == &rhs) {
    return true;
  }
  if (this->ascii->size() != rhs.ascii->size()) {
    return false;
  }
  if ((this->canonical || this->extlang_form) &&
      (rhs.canonical || rhs.extlang_form)) {
    return memcmp(this->ascii->dptr(), rhs.ascii->dptr(), rhs.ascii->size())
           == 0;
  }
  uint8_t *begin = this->ascii->dptr();
  uint8_t *end = begin + this->ascii->size();
  uint8_t *mark = rhs.ascii->dptr();
  for (; begin != end; ++begin) {
    if (to_lower(*begin) != to_lower(*mark)) {
      return false;
    }
    ++mark;
  }
  return true;
}

bool LangTag::operator!=(const LangTag &rhs) throw() {
  return !(*this == rhs);
}

const uint8_t *LangTag::lookup(const uint8_t *key,
                               size_t len,
                               const void **values,
                               size_t &outlen) throw() {
  if (values == NULL) {
    return NULL;
  }
  vector<uint32_t> keylist;
  size_t i;
  for (i = 0; i < len; i += 5) {
    size_t l = len - i;
    if (l > 5) {
      l = 5;
    }
    uint32_t k = 0;
    size_t j;
    for (j = 0; j < l; ++j) {
      k = (k << 6) | (uint32_t) LANG_CHAR_ENCODE(key[i + j]);
    }
    k <<= 2;
    keylist.push_back(k);
  }

  const uint32_t *keys = (const uint32_t *)values[0];
  ++values;
  uint32_t nk = keys[0];
  ++keys;
  vector<uint32_t>::iterator it;
  for (it = keylist.begin(); it != keylist.end(); ++it) {
    const uint32_t *kfound = lower_bound(keys, keys + nk, *it);
    // no leaf or branch
    if (kfound == keys + nk) {
      return NULL;
    }
    // found leaf
    if (*it == *kfound && it + 1 == keylist.end()) {
      const uint8_t *found = (const uint8_t *) values[kfound - keys];
      outlen = found[0];
      return found + 1;
    }
    // no branch
    if ((*it | UINT32_C(0x1)) != *kfound) {
      return NULL;
    }
    // found branch
    values = (const void **) values[kfound - keys];
    keys = (const uint32_t *) values[0];
    ++values;
    nk = keys[0];
    ++keys;
  }
  return NULL;
}

void LangTag::normalizePart(enum LangTagPart p)
    THROWS(BadAllocException) {
  DPtr<uint8_t> *part = this->getPart(p);

  if (part == NULL) {
    return;
  }

  if (p == EXTENSIONS || p == PRIVATE_USE) {
    uint8_t *begin = part->dptr();
    uint8_t *end = begin + part->size();
    for (; begin != end; ++begin) {
      *begin = to_lower(*begin);
    }
    part->drop();
    return;
  }

  if (p == LANGUAGE) {
    uint8_t *begin = part->dptr();
    uint8_t *end = begin + part->size();
    uint8_t *mark = begin;
    for (; mark != end; ++mark) {
      if (*mark == to_ascii('-')) {
        begin = mark + 1;
      }
    }
    size_t offset = begin - part->dptr();
    if (offset > 0) {
      DPtr<uint8_t> *s = this->ascii->sub(offset,
                                          this->ascii->size() - offset);
      this->ascii->drop();
      this->ascii = s;
    }
  }

  // Maybe ordering variants according to http://tools.ietf.org/rfc/rfc5646.txt
  // would be appropriate, but then so many more arrays and extra logic would
  // be required for a relatively rare case.  For now, it is considered
  // prohibitive to performance outweighing potential gain in interoperability.

  const void **lookup_array = NULL;
  switch (p) {
    case LANGUAGE:
      lookup_array = LANG_LANGUAGE_SUBTAG_PREFERRED_VALUE_VALUES;
      break;
    case SCRIPT:
      lookup_array = LANG_SCRIPT_SUBTAG_PREFERRED_VALUE_VALUES;
      break;
    case REGION:
      lookup_array = LANG_REGION_SUBTAG_PREFERRED_VALUE_VALUES;
      break;
    case VARIANTS:
      lookup_array = LANG_VARIANT_SUBTAG_PREFERRED_VALUE_VALUES;
      break;
  }

  if (this->replaceSection(part, lookup_array)) {
    part->drop();
    return;
  }

  uint8_t *begin = part->dptr();
  uint8_t *end = begin + part->size();
  switch (p) {
    case REGION:
      for (; begin != end; ++begin) {
        *begin = to_upper(*begin);
      }
      break;
    case SCRIPT:
      *begin = to_upper(*begin);
      ++begin;
      // fall-through
    case LANGUAGE:
    case VARIANTS:
      for (; begin != end; ++begin) {
        *begin = to_lower(*begin);
      }
      break;
  }
  part->drop();
}
TRACE(BadAllocException, "(trace)")

bool LangTag::replaceSection(DPtr<uint8_t> *part, const void **lookup_array)
    THROWS(BadAllocException) {
  size_t newlen;
  const uint8_t *replacement = lookup(part->dptr(), part->size(),
      lookup_array, newlen);
  if (replacement == NULL) {
    return false;
  }
  uint8_t *begin = part->dptr();
  uint8_t *end = begin + part->size();
  if (newlen <= part->size()) {
    memcpy(begin, replacement, newlen * sizeof(uint8_t));
    if (newlen < part->size()) {
      size_t taillen = this->ascii->size() - (end - this->ascii->dptr());
      memmove(begin + newlen, end, taillen);
      DPtr<uint8_t> *s = this->ascii->sub(0,
          this->ascii->size() - part->size() + newlen);
      this->ascii->drop();
      this->ascii = s;
    }
  } else {
    size_t frontlen = begin - this->ascii->dptr();
    size_t taillen = this->ascii->size() - frontlen - part->size();
    size_t totallen = frontlen + newlen + taillen;
    DPtr<uint8_t> *s;
    NEW(s, MPtr<uint8_t>, totallen);
    memcpy(s->dptr(), this->ascii->dptr(), frontlen * sizeof(uint8_t));
    memcpy(s->dptr() + frontlen, begin, newlen * sizeof(uint8_t));
    memcpy(s->dptr() + frontlen + newlen, end, taillen * sizeof(uint8_t));
    this->ascii->drop();
    this->ascii = s;
  }
  return true;
}
TRACE(BadAllocException, "(trace)")

}
