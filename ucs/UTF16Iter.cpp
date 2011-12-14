#include "ucs/UTF16Iter.h"

#include "ptr/MPtr.h"
#include "ucs/utf.h"

namespace ucs {

UTF16Iter::UTF16Iter() throw(BadAllocException)
    : UCSIter(), utf16str(NULL) {
  try {
    NEW(this->utf16str, MPtr<uint16_t>);
  } JUST_RETHROW(BadAllocException, "(rethrow)")
  this->flip = false;
  this->marker = NULL;
  this->reset_mark = NULL;
  this->value = 0;
  this->reset_value = 0;
}

UTF16Iter::UTF16Iter(DPtr<uint16_t> *utf16str) throw(SizeUnknownException)
    : UCSIter(), utf16str(utf16str) {
  if (!this->utf16str->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  this->utf16str->hold();
  if (this->utf16str->size() <= 0) {
    this->flip = false;
    this->marker = NULL;
    this->reset_mark = NULL;
    this->value = 0;
    this->reset_value = 0;
  } else {
    this->flip = utf16flip(this->utf16str, &(this->marker));
    if (this->utf16str->dptr() + this->utf16str->size() == this->marker) {
      this->marker = NULL;
      this->value = 0;
    } else {
      this->value = utf16char(this->marker, this->flip, &(this->marker));
    }
    this->reset_mark = this->marker;
    this->reset_value = this->value;
  }
}

UTF16Iter::UTF16Iter(const UTF16Iter &copy)
    : UCSIter() {
  this->utf16str = copy.utf16str;
  this->utf16str->hold();
  this->marker = copy.marker;
  this->flip = copy.flip;
  this->value = copy.value;
  this->reset_mark = copy.reset_mark;
  this->reset_value = copy.reset_value;
}

UTF16Iter::~UTF16Iter() {
  this->utf16str->drop();
}

UTF16Iter *UTF16Iter::begin(DPtr<uint16_t> *utf16str) {
  UTF16Iter *iter;
  NEW(iter, UTF16Iter, utf16str);
  return iter;
}

UTF16Iter *UTF16Iter::end(DPtr<uint16_t> *utf16str) {
  UTF16Iter *iter;
  NEW(iter, UTF16Iter, utf16str);
  return (UTF16Iter *) iter->finish();
}

UCSIter *UTF16Iter::start() {
  if (this->utf16str->size() > 0) {
    this->flip = utf16flip(this->utf16str, &(this->marker));
    this->value = utf16char(this->marker, this->flip, &(this->marker));
    if (this->marker == this->utf16str->dptr() + this->utf16str->size()) {
      this->marker = NULL;
    }
  }
  return this;
}

UCSIter *UTF16Iter::finish() {
  this->marker = NULL;
  return this;
}

uint32_t UTF16Iter::current() {
  return this->value;
}

UCSIter *UTF16Iter::advance() {
  if (this->marker == NULL) {
    return NULL;
  }
  if (this->marker == this->utf16str->dptr() + this->utf16str->size()) {
    this->marker = NULL;
  } else {
    this->value = utf16char(this->marker, this->flip, &(this->marker));
  }
  return this;
}

bool UTF16Iter::more() {
  return this->marker != NULL;
}

void UTF16Iter::mark() {
  this->reset_mark = this->marker;
  this->reset_value = this->value;
}

void UTF16Iter::reset() {
  this->marker = this->reset_mark;
  this->value = this->reset_value;
}

UTF16Iter &UTF16Iter::operator=(UTF16Iter &rhs) {
  if (this != &rhs) {
    this->utf16str->drop();
    this->utf16str = rhs.utf16str;
    this->utf16str->hold();
    this->flip = rhs.flip;
    this->marker = rhs.marker;
    this->value = rhs.value;
    this->reset_mark = rhs.reset_mark;
    this->reset_value = rhs.reset_value;
  }
  return *this;
}

bool UTF16Iter::operator==(UTF16Iter &rhs) {
  return this->marker == rhs.marker &&
      this->utf16str->dptr() == rhs.utf16str->dptr() &&
      this->utf16str->size() == rhs.utf16str->size() &&
      (this->marker == NULL || this->value == rhs.value) &&
      (this->flip == rhs.flip);
}

bool UTF16Iter::operator!=(UTF16Iter &rhs) {
  return !(*this == rhs);
}

uint32_t UTF16Iter::operator*() {
  return this->current();
}

UTF16Iter &UTF16Iter::operator++() {
  return *((UTF16Iter*)(this->advance()));
}

UTF16Iter UTF16Iter::operator++(int) {
  UTF16Iter ret = *this;
  this->advance();
  return ret;
}

}
