#include "ucs/UTF16Iter.h"

#include "ptr/MPtr.h"
#include "ucs/utf.h"

namespace ucs {

using namespace ptr;

UTF16Iter::UTF16Iter(DPtr<uint16_t> *utf16str) throw(SizeUnknownException)
    : UCSIter(), utf16str(utf16str), flip(false), marker(NULL),
      reset_mark(NULL), value(0), reset_value(0) {
  if (!this->utf16str->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  this->utf16str->hold();
  if (this->utf16str->size() > 0) {
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

}
