#include "ucs/UCSIter.h"

#include "sys/ints.h"
#include "ucs/utf.h"

namespace ucs {

using namespace std;

inline
UCSIter::UCSIter()
    : validate_codepoints(false) {
  // do nothing
}

inline
UCSIter::~UCSIter() {
  // do nothing
}

inline
void UCSIter::validate(const bool v) {
  this->validate_codepoints = v;
}

inline
size_t UCSIter::current(uint8_t *utf8) {
  return utf8len(this->current(), utf8);
}

inline
size_t UCSIter::current(uint16_t *utf16, const enum BOM bom) {
  return utf16len(this->current(), bom, utf16);
}

inline
size_t UCSIter::current(uint32_t *utf32, const enum BOM bom) {
  return utf32len(this->current(), bom, utf32);
}

inline
uint32_t UCSIter::next() {
  uint32_t codepoint = this->current();
  this->advance();
  return codepoint;
}

inline
size_t UCSIter::next(uint8_t *utf8) {
  size_t len = this->current(utf8);
  this->advance();
  return len;
}

inline
size_t UCSIter::next(uint16_t *utf16, const enum BOM bom) {
  size_t len = this->current(utf16, bom);
  this->advance();
  return len;
}

inline
size_t UCSIter::next(uint32_t *utf32, const enum BOM bom) {
  size_t len = this->current(utf32, bom);
  this->advance();
  return len;
}

}
