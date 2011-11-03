#include "ucs/UTF8Iter.h"

#include "ucs/utf.h"

namespace ucs {

UTF8Iter::UTF8Iter(DPtr<uint8_t> *utf8str) throw(SizeUnknownException)
    : UCSIter(), utf8str(utf8str) {
  if (!this->utf8str->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  this->utf8str->hold();
  if (this->utf8str->size() <= 0) {
    this->mark = NULL;
  } else {
    this->value = utf8char(this->utf8str->dptr(), &(this->mark));
  }
}

UTF8Iter::UTF8Iter(const UTF8Iter &copy)
    : UCSIter() {
  this->utf8str = copy.utf8str;
  this->utf8str->hold();
  this->mark = copy.mark;
  this->value = copy.value;
}

UTF8Iter::~UTF8Iter() {
  this->utf8str->drop();
}

UTF8Iter *UTF8Iter::begin(DPtr<uint8_t> *utf8str) {
  return new UTF8Iter(utf8str);
}

UTF8Iter *UTF8Iter::end(DPtr<uint8_t> *utf8str) {
  return (UTF8Iter *)((new UTF8Iter(utf8str))->finish());
}

UCSIter *UTF8Iter::start() {
  this->mark = this->utf8str->dptr();
  this->value = utf8char(this->mark, &(this->mark));
  return this;
}

UCSIter *UTF8Iter::finish() {
  this->mark = NULL;
  return this;
}

uint32_t UTF8Iter::current() {
  return this->value;
}

UCSIter *UTF8Iter::advance() {
  if (this->mark == NULL) {
    return NULL;
  }
  if (this->mark == this->utf8str->dptr() + this->utf8str->size()) {
    this->mark = NULL;
  } else {
    this->value = utf8char(this->mark, &(this->mark));
  }
  return this;
}

bool UTF8Iter::more() {
  return this->mark != NULL;
}

UTF8Iter &UTF8Iter::operator=(UTF8Iter &rhs) {
  if (this != &rhs) {
    this->utf8str->drop();
    this->utf8str = rhs.utf8str;
    this->utf8str->hold();
    this->mark = rhs.mark;
    this->value = rhs.value;
  }
  return *this;
}

bool UTF8Iter::operator==(UTF8Iter &rhs) {
  return this->mark == rhs.mark &&
      this->utf8str->dptr() == rhs.utf8str->dptr() &&
      this->utf8str->size() == rhs.utf8str->size() &&
      (this->mark == NULL || this->value == rhs.value);
}

bool UTF8Iter::operator!=(UTF8Iter &rhs) {
  return !(*this == rhs);
}

uint32_t UTF8Iter::operator*() {
  return this->current();
}

UTF8Iter &UTF8Iter::operator++() {
  return *((UTF8Iter*)(this->advance()));
}

}
