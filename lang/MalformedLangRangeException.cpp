#include "lang/MalformedLangRangeException.h"

#include <string>
#include <iostream>
#include "ucs/utf.h"

namespace lang {

using namespace std;

MalformedLangRangeException::MalformedLangRangeException(const char *file,
    const unsigned int line) throw()
    : TraceableException(file, line, "Malformed LangRange") {
  // do nothing
}

MalformedLangRangeException::MalformedLangRangeException(const char *file,
    const unsigned int line, DPtr<uint8_t> *mal) throw()
    : TraceableException(file, line, "Malformed LangRange") {
  if (mal != NULL) {
    this->stack_trace.push_back(string((char*) mal->ptr(), mal->size()));
  }
}

MalformedLangRangeException::~MalformedLangRangeException() throw() {
  // do nothing
}

const char *MalformedLangRangeException::what() const throw() {
  return TraceableException::what();
}

}
