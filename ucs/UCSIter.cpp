#include "ucs/UCSIter.h"

#include "sys/ints.h"
#include "ucs/utf.h"

namespace ucs {

using namespace std;

UCSIter::UCSIter() {
  // do nothing
}

UCSIter::~UCSIter() {
  // do nothing
}

size_t UCSIter::current(uint8_t *utf8) {
  return utf8len(this->current(), utf8);
}

size_t UCSIter::current(uint16_t *utf16, const enum BOM bom) {
  return utf16len(this->current(), bom, utf16);
}

size_t UCSIter::current(uint32_t *utf32, const enum BOM bom) {
  return utf32len(this->current(), bom, utf32);
}

uint32_t UCSIter::next() {
  uint32_t codepoint = this->current();
  this->advance();
  return codepoint;
}

size_t UCSIter::next(uint8_t *utf8) {
  size_t len = this->current(utf8);
  this->advance();
  return len;
}

size_t UCSIter::next(uint16_t *utf16, const enum BOM bom) {
  size_t len = this->current(utf16, bom);
  this->advance();
  return len;
}

size_t UCSIter::next(uint32_t *utf32, const enum BOM bom) {
  size_t len = this->current(utf32, bom);
  this->advance();
  return len;
}

}
