#include "ucs/UTF8Iter.h"

#include "ptr/MPtr.h"
#include "ucs/InvalidCodepointException.h"
#include "ucs/nf.h"
#include "ucs/utf.h"

namespace ucs {

using namespace ptr;

UTF8Iter::UTF8Iter(DPtr<uint8_t> *utf8str) throw(SizeUnknownException)
    : UCSIter(), utf8str(utf8str), marker(NULL), reset_mark(NULL), value(0),
      reset_value(0) {
  if (!this->utf8str->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  this->utf8str->hold();
  if (this->utf8str->size() > 0) {
    this->value = utf8char(this->utf8str->dptr(), &(this->marker));
    this->reset_mark = this->marker;
    this->reset_value = this->value;
  }
}

UCSIter *UTF8Iter::start() {
  if (this->utf8str->size() > 0) {
    this->marker = this->utf8str->dptr();
    this->value = utf8char(this->marker, &(this->marker));
    if (this->validate_codepoints && !nfvalid(this->value)) {
      THROW(InvalidCodepointException, this->value);
    }
  }
  return this;
}

UCSIter *UTF8Iter::advance() {
  if (this->marker == NULL) {
    return NULL;
  }
  if (this->marker == this->utf8str->dptr() + this->utf8str->size()) {
    this->marker = NULL;
  } else {
    this->value = utf8char(this->marker, &(this->marker));
    if (this->validate_codepoints && !nfvalid(this->value)) {
      THROW(InvalidCodepointException, this->value);
    }
  }
  return this;
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
    this->validate_codepoints = rhs.validate_codepoints;
  }
  return *this;
}

}
