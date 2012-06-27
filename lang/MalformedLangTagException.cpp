#include "lang/MalformedLangTagException.h"

#include <string>
#include <iostream>
#include "ucs/utf.h"

namespace lang {

using namespace std;

MalformedLangTagException::MalformedLangTagException(const char *file,
    const unsigned int line) throw()
    : TraceableException(file, line, "Malformed LangTag") {
  // do nothing
}

MalformedLangTagException::MalformedLangTagException(const char *file,
    const unsigned int line, DPtr<uint8_t> *mal) throw()
    : TraceableException(file, line, "Malformed LangTag") {
  if (mal != NULL) {
    this->amendStackTrace(file, line,
                          string((char*) mal->ptr(), mal->size()).c_str());
  }
}

MalformedLangTagException::~MalformedLangTagException() throw() {
  // do nothing
}

const char *MalformedLangTagException::what() const throw() {
  return TraceableException::what();
}

}
