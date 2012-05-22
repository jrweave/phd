#include "io/OutputStream.h"

namespace io {

OutputStream::~OutputStream() throw(IOException) {
  // do nothing
}

void OutputStream::write(DPtr<uint8_t> *buf)
    throw(IOException, SizeUnknownException, BaseException<void*>) {
  size_t nwritten;
  try {
    this->write(buf, nwritten);
  } JUST_RETHROW(IOException, "(rethrow)")
    JUST_RETHROW(SizeUnknownException, "(rethrow)")
    JUST_RETHROW(BaseException<void*>, "(rethrow)")
}

}
