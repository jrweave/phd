#include "io/OFStream.h"

namespace io {

using namespace std;

inline
OFStream::OFStream(const char *filename) throw()
    : OStream<ofstream>() {
  this->stream.open(filename, ofstream::out | ofstream::binary);
  if (this->stream.fail()) {
    THROW(IOException, "Failed to open file.");
  }
}

inline
OFStream::~OFStream() throw(IOException) {
  // do nothing
}

inline
void OFStream::close() throw(IOException) {
  this->stream.close();
  if (this->stream.fail()) {
    THROW(IOException, "Failed to close ofstream.");
  }
}

}
