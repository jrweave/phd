#include "ucs/UTF8Iter.h"

#include "ptr/MPtr.h"
#include "ucs/utf.h"

namespace ucs {

UTF8Iter::UTF8Iter() throw(BadAllocException)
    : UCSIter(), utf8str(NULL) {
  try {
    NEW(this->utf8str, MPtr<uint8_t>);
  } JUST_RETHROW(BadAllocException, "(rethrow)")
  this->marker = NULL;
  this->reset_mark = NULL;
  this->value = 0;
  this->reset_value = 0;
}

UTF8Iter::UTF8Iter(DPtr<uint8_t> *utf8str) throw(SizeUnknownException)
    : UCSIter(), utf8str(utf8str) {
  if (!this->utf8str->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  this->utf8str->hold();
  if (this->utf8str->size() <= 0) {
    this->marker = NULL;
    this->reset_mark = NULL;
    this->value = 0;
    this->reset_value = 0;
  } else {
    this->value = utf8char(this->utf8str->dptr(), &(this->marker));
    this->reset_mark = this->marker;
    this->reset_value = this->value;
  }
}

UTF8Iter::UTF8Iter(const UTF8Iter &copy)
    : UCSIter() {
  this->utf8str = copy.utf8str;
  this->utf8str->hold();
  this->marker = copy.marker;
  this->value = copy.value;
  this->reset_mark = copy.reset_mark;
  this->reset_value = copy.reset_value;
}

UTF8Iter::~UTF8Iter() {
  this->utf8str->drop();
}

UTF8Iter *UTF8Iter::begin(DPtr<uint8_t> *utf8str) {
  UTF8Iter *iter;
  NEW(iter, UTF8Iter, utf8str);
  return iter;
}

UTF8Iter *UTF8Iter::end(DPtr<uint8_t> *utf8str) {
  UTF8Iter *iter;
  NEW(iter, UTF8Iter, utf8str);
  return (UTF8Iter *)iter->finish();
}

UCSIter *UTF8Iter::start() {
  if (this->utf8str->size() > 0) {
    this->marker = this->utf8str->dptr();
    this->value = utf8char(this->marker, &(this->marker));
  }
  return this;
}

UCSIter *UTF8Iter::finish() {
  this->marker = NULL;
  return this;
}

uint32_t UTF8Iter::current() {
  return this->value;
}

UCSIter *UTF8Iter::advance() {
  if (this->marker == NULL) {
    return NULL;
  }
  if (this->marker == this->utf8str->dptr() + this->utf8str->size()) {
    this->marker = NULL;
  } else {
    this->value = utf8char(this->marker, &(this->marker));
  }
  return this;
}

bool UTF8Iter::more() {
  return this->marker != NULL;
}

void UTF8Iter::mark() {
  this->reset_mark = this->marker;
  this->reset_value = this->value;
}

void UTF8Iter::reset() {
  this->marker = this->reset_mark;
  this->value = this->reset_value;
}

UTF8Iter &UTF8Iter::operator=(UTF8Iter &rhs) {
  if (this != &rhs) {
    this->utf8str->drop();
    this->utf8str = rhs.utf8str;
    this->utf8str->hold();
    this->marker = rhs.marker;
    this->value = rhs.value;
    this->reset_mark = rhs.reset_mark;
    this->reset_value = rhs.reset_value;
  }
  return *this;
}

bool UTF8Iter::operator==(UTF8Iter &rhs) {
  return this->marker == rhs.marker &&
      this->utf8str->dptr() == rhs.utf8str->dptr() &&
      this->utf8str->size() == rhs.utf8str->size() &&
      (this->marker == NULL || this->value == rhs.value);
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

UTF8Iter UTF8Iter::operator++(int) {
  UTF8Iter ret = *this;
  this->advance();
  return ret;
}

}
