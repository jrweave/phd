#include "ucs/UTF16Iter.h"

#include "ucs/utf.h"

namespace ucs {

UTF16Iter::UTF16Iter(DPtr<uint16_t> *utf16str) throw(SizeUnknownException)
    : UCSIter(), utf16str(utf16str) {
  if (!this->utf16str->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  this->utf16str->hold();
  if (this->utf16str->size() <= 0) {
    this->flip = false;
    this->mark = NULL;
  } else {
    this->flip = utf16flip(this->utf16str, &(this->mark));
    this->value = utf16char(this->mark, this->flip, &(this->mark));
  }
}

UTF16Iter::UTF16Iter(const UTF16Iter &copy)
    : UCSIter() {
  this->utf16str = copy.utf16str;
  this->utf16str->hold();
  this->mark = copy.mark;
  this->flip = copy.flip;
  this->value = copy.value;
}

UTF16Iter::~UTF16Iter() {
  this->utf16str->drop();
}

UTF16Iter *UTF16Iter::begin(DPtr<uint16_t> *utf16str) {
  return new UTF16Iter(utf16str);
}

UTF16Iter *UTF16Iter::end(DPtr<uint16_t> *utf16str) {
  return (UTF16Iter *)((new UTF16Iter(utf16str))->finish());
}

UCSIter *UTF16Iter::start() {
  this->flip = utf16flip(this->utf16str, &(this->mark));
  this->value = utf16char(this->mark, this->flip, &(this->mark));
  return this;
}

UCSIter *UTF16Iter::finish() {
  this->mark = NULL;
  return this;
}

uint32_t UTF16Iter::current() {
  return this->value;
}

UCSIter *UTF16Iter::advance() {
  if (this->mark == NULL) {
    return NULL;
  }
  if (this->mark == this->utf16str->dptr() + this->utf16str->size()) {
    this->mark = NULL;
  } else {
    this->value = utf16char(this->mark, this->flip, &(this->mark));
  }
  return this;
}

bool UTF16Iter::more() {
  return this->mark != NULL;
}

UTF16Iter &UTF16Iter::operator=(UTF16Iter &rhs) {
  if (this != &rhs) {
    this->utf16str->drop();
    this->utf16str = rhs.utf16str;
    this->utf16str->hold();
    this->flip = rhs.flip;
    this->mark = rhs.mark;
    this->value = rhs.value;
  }
  return *this;
}

bool UTF16Iter::operator==(UTF16Iter &rhs) {
  return this->mark == rhs.mark &&
      this->utf16str->dptr() == rhs.utf16str->dptr() &&
      this->utf16str->size() == rhs.utf16str->size() &&
      (this->mark == NULL || this->value == rhs.value) &&
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

}
