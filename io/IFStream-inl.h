#include "io/IFStream.h"

namespace io {

inline
IFStream::IFStream(const char *filename)
    throw(BadAllocException, IOException)
    : IStream<ifstream>(4096) {
  this->stream.open(filename, ifstream::in | ifstream::binary);
  if (this->stream.fail()) {
    THROW(IOException, "Failed to open file.");
  }
}

inline
IFStream::IFStream(const char *filename, const size_t bufsize)
    throw(BadAllocException, IOException)
    : IStream<ifstream>(bufsize) {
  this->stream.open(filename, ifstream::in | ifstream::binary);
  if (this->stream.fail()) {
    THROW(IOException, "Failed to open file.");
  }
}

inline
IFStream::~IFStream() throw(IOException) {
  // do nothing; superclass destructor closes
}

}
