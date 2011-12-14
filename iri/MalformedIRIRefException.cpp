#include "iri/MalformedIRIRefException.h"

#include <string>
#include <iostream>
#include <sstream>
#include "ucs/utf.h"

namespace iri {

using namespace std;

MalformedIRIRefException::MalformedIRIRefException(const char *file,
    const unsigned int line) throw()
    : TraceableException(file, line, "Malformed IRIRef") {
  // do nothing
}

MalformedIRIRefException::MalformedIRIRefException(const char *file,
    const unsigned int line, UCSIter *mal) throw()
    : TraceableException(file, line, "Malformed IRIRef") {
  if (mal != NULL) {
    string str;
    uint8_t buf[4];
    mal->start();
    while (mal->more()) {
      size_t len = mal->next(buf);
      size_t i;
      for (i = 0; i < len; i++) {
        str.push_back((char) buf[i]);
      }
    }
    this->stack_trace.push_back(str);
    DELETE(mal);
  }
}

MalformedIRIRefException::~MalformedIRIRefException() throw() {
  // do nothing
}

const char *MalformedIRIRefException::what() const throw() {
  return TraceableException::what();
}

}
