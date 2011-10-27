#include "ptr/SizeUnknownException.h"

#include <sstream>

namespace ptr {

using namespace std;

SizeUnknownException::SizeUnknownException(const char *file,
    const unsigned int line) throw()
    : TraceableException(file, line, "Unknown size.") {
  // do nothing
}

SizeUnknownException::~SizeUnknownException() throw() {
  // do nothing
}

}
