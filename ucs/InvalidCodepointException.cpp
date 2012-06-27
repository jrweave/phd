#include "ucs/InvalidCodepointException.h"

#include <sstream>

namespace ucs {

using namespace ex;
using namespace std;

InvalidCodepointException::InvalidCodepointException(const char *file,
    const unsigned int line, const uint32_t codepoint) throw()
    : TraceableException(file, line, "Invalid codepoint."),
      codepoint(codepoint) {
  stringstream ss (stringstream::in | stringstream::out);
  ss << TraceableException::what() << " caused by 0x"
      << hex << this->codepoint << "\n";
  this->stack_trace.append(ss.str());
}

InvalidCodepointException::~InvalidCodepointException() throw() {
  // do nothing
}

}
