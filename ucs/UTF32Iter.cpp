#include "ucs/UTF32Iter.h"

#include "ucs/utf.h"

namespace ucs {

UTF32Iter::UTF32Iter(DPtr<uint32_t> *utf32str) throw(SizeUnknownException)
    : UCSIter(), utf32str(utf32str) {
  if (!this->utf32str->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  this->utf32str->hold();
  if (this->utf32str->size() <= 0) {
    this->flip = false;
    this->mark = NULL;
  } else {
    this->flip = utf32flip(this->utf32str, &(this->mark));
    this->value = utf32char(this->mark, this->flip, &(this->mark));
  }
}

UTF32Iter::UTF32Iter(const UTF32Iter &copy)
    : UCSIter() {
  this->utf32str = copy.utf32str;
  this->utf32str->hold();
  this->mark = copy.mark;
  this->flip = copy.flip;
  this->value = copy.value;
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
  this->flip = utf32flip(this->utf32str, &(this->mark));
  this->value = utf32char(this->mark, this->flip, &(this->mark));
  return this;
}

UCSIter *UTF32Iter::finish() {
  this->mark = NULL;
  return this;
}

uint32_t UTF32Iter::current() {
  return this->value;
}

UCSIter *UTF32Iter::advance() {
  if (this->mark == NULL) {
    return NULL;
  }
  if (this->mark == this->utf32str->dptr() + this->utf32str->size()) {
    this->mark = NULL;
  } else {
    this->value = utf32char(this->mark, this->flip, &(this->mark));
  }
  return this;
}

bool UTF32Iter::more() {
  return this->mark != NULL;
}

UTF32Iter &UTF32Iter::operator=(UTF32Iter &rhs) {
  if (this != &rhs) {
    this->utf32str->drop();
    this->utf32str = rhs.utf32str;
    this->utf32str->hold();
    this->flip = rhs.flip;
    this->mark = rhs.mark;
    this->value = rhs.value;
  }
  return *this;
}

bool UTF32Iter::operator==(UTF32Iter &rhs) {
  return this->mark == rhs.mark &&
      this->utf32str->dptr() == rhs.utf32str->dptr() &&
      this->utf32str->size() == rhs.utf32str->size() &&
      (this->mark == NULL || this->value == rhs.value) &&
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

}
