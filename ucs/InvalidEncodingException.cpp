#include "ucs/InvalidEncodingException.h"

#include <sstream>
#include "sys/ints.h"

namespace ucs {

using namespace ex;
using namespace std;

InvalidEncodingException::InvalidEncodingException(const char *file,
    const unsigned int line, const char *message, const uint32_t encoded)
    throw()
    : TraceableException(file, line, message), encoded(encoded) { 
  stringstream ss (stringstream::in | stringstream::out);
  ss << " caused by invalid encoding 0x" << hex << this->encoded << "\n";
  this->stack_trace.append(ss.str());
}

InvalidEncodingException::~InvalidEncodingException() throw() {
  // do nothing
}

}
