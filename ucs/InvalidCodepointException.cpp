#include "ucs/InvalidCodepointException.h"

#include <sstream>

namespace ucs {

using namespace ex;
using namespace std;

InvalidCodepointException::InvalidCodepointException(const char *file,
    const unsigned int line, const uint32_t codepoint) throw()
    : TraceableException(file, line), codepoint(codepoint) {
  // do nothing
}

InvalidCodepointException::~InvalidCodepointException() throw() {
  // do nothing
}

const char *InvalidCodepointException::what() const throw() {
  stringstream ss (stringstream::in | stringstream::out);
  ss << TraceableException::what() << "\tcaused by invalid codepoint 0x"
      << hex << this->codepoint << "\n";
  return ss.str().c_str();
}

}
