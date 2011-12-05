#include "ucs/UTF32Iter.h"

#include "ptr/MPtr.h"
#include "ucs/utf.h"

namespace ucs {

UTF32Iter::UTF32Iter() throw(BadAllocException)
    : UCSIter(), utf32str(NULL) {
  try {
    this->utf32str = new MPtr<uint32_t>();
  } JUST_RETHROW(BadAllocException, "(rethrow)")
  this->flip = false;
  this->marker = NULL;
  this->reset_mark = NULL;
  this->value = 0;
  this->reset_value = 0;
}

UTF32Iter::UTF32Iter(DPtr<uint32_t> *utf32str) throw(SizeUnknownException)
    : UCSIter(), utf32str(utf32str) {
  if (!this->utf32str->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  this->utf32str->hold();
  if (this->utf32str->size() <= 0) {
    this->flip = false;
    this->marker = NULL;
    this->reset_mark = NULL;
    this->value = 0;
    this->reset_value = 0;
  } else {
    this->flip = utf32flip(this->utf32str, &(this->marker));
    if (this->utf32str->dptr() + this->utf32str->size() == this->marker) {
      this->marker = NULL;
      this->value = 0;
    } else {
      this->value = utf32char(this->marker, this->flip, &(this->marker));
    }
    this->reset_mark = this->marker;
    this->reset_value = this->value;
  }
}

UTF32Iter::UTF32Iter(const UTF32Iter &copy)
    : UCSIter() {
  this->utf32str = copy.utf32str;
  this->utf32str->hold();
  this->marker = copy.marker;
  this->flip = copy.flip;
  this->value = copy.value;
  this->reset_mark = copy.reset_mark;
  this->reset_value = copy.reset_value;
}

UTF32Iter::~UTF32Iter() {
  this->utf32str->drop();
}

UTF32Iter *UTF32Iter::begin(DPtr<uint32_t> *utf32str) {
  return new UTF32Iter(utf32str);
}

UTF32Iter *UTF32Iter::end(DPtr<uint32_t> *utf32str) {
  return (UTF32Iter *)((new UTF32Iter(utf32str))->finish());
}

UCSIter *UTF32Iter::start() {
  if (this->utf32str->size() > 0) {
    this->flip = utf32flip(this->utf32str, &(this->marker));
    this->value = utf32char(this->marker, this->flip, &(this->marker));
    if (this->marker == this->utf32str->dptr() + this->utf32str->size()) {
      this->marker = NULL;
    }
  }
  return this;
}

UCSIter *UTF32Iter::finish() {
  this->marker = NULL;
  return this;
}

uint32_t UTF32Iter::current() {
  return this->value;
}

UCSIter *UTF32Iter::advance() {
  if (this->marker == NULL) {
    return NULL;
  }
  if (this->marker == this->utf32str->dptr() + this->utf32str->size()) {
    this->marker = NULL;
  } else {
    this->value = utf32char(this->marker, this->flip, &(this->marker));
  }
  return this;
}

bool UTF32Iter::more() {
  return this->marker != NULL;
}

void UTF32Iter::mark() {
  this->reset_mark = this->marker;
  this->reset_value = this->value;
}

void UTF32Iter::reset() {
  this->marker = this->reset_mark;
  this->value = this->reset_value;
}

UTF32Iter &UTF32Iter::operator=(UTF32Iter &rhs) {
  if (this != &rhs) {
    this->utf32str->drop();
    this->utf32str = rhs.utf32str;
    this->utf32str->hold();
    this->flip = rhs.flip;
    this->marker = rhs.marker;
    this->value = rhs.value;
    this->reset_mark = rhs.reset_mark;
    this->reset_value = rhs.reset_value;
  }
  return *this;
}

bool UTF32Iter::operator==(UTF32Iter &rhs) {
  return this->marker == rhs.marker &&
      this->utf32str->dptr() == rhs.utf32str->dptr() &&
      this->utf32str->size() == rhs.utf32str->size() &&
      (this->marker == NULL || this->value == rhs.value) &&
      (this->flip == rhs.flip);
}

bool UTF32Iter::operator!=(UTF32Iter &rhs) {
  return !(*this == rhs);
}

uint32_t UTF32Iter::operator*() {
  return this->current();
}

UTF32Iter &UTF32Iter::operator++() {
  return *((UTF32Iter*)(this->advance()));
}

UTF32Iter UTF32Iter::operator++(int) {
  UTF32Iter ret = *this;
  this->advance();
  return ret;
}

}
