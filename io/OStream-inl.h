#include "io/OStream.h"

namespace io {

template<typename ostream_t>
OStream<ostream_t>::OStream() throw()
    : OutputStream() {
  // do nothing
}

template<typename ostream_t>
OStream<ostream_t>::OStream(ostream_t &stream) throw()
    : OutputStream(), stream(stream) {
  // do nothing
}

template<typename ostream_t>
OStream<ostream_t>::~OStream() throw(IOException) {
  // do nothing
}

template<typename ostream_t>
void OStream<ostream_t>::close() throw(IOException) {
  // ostream has no close method;
  // at least flush the buffer
  this->stream.flush();
  if (this->stream.bad()) {
    THROW(IOException, "Failed to flush ostream when close was called.");
  }
}

template<typename ostream_t>
void OStream<ostream_t>::flush() throw(IOException) {
  this->stream.flush();
  if (this->stream.bad()) {
    THROW(IOException, "Failed to flush ostream.");
  }
}

template<typename ostream_t>
void OStream<ostream_t>::write(DPtr<uint8_t> *buf, size_t &nwritten)
    throw(IOException, SizeUnknownException, BaseException<void*>) {
  if (buf == NULL) {
    THROW(BaseException<void*>, NULL, "buf must not be NULL.");
  }
  if (!buf->sizeKnown()) {
    THROWX(SizeUnknownException);
  }
  const uint8_t *mark = buf->dptr();
  const uint8_t *end = mark + buf->size();
  for (; mark != end; ++mark) {
    this->stream.put(to_lchar(*mark));
    if (this->stream.bad()) {
      nwritten = mark - buf->dptr();
      THROW(IOException, "Error when writing to underlying stream.");
    }
  }
  nwritten = mark - buf->dptr();
}

template<typename ostream_t>
inline
streampos OStream<ostream_t>::tellp() throw(IOException) {
  return this->stream.tellp();
}

template<typename ostream_t>
void OStream<ostream_t>::seekp(streampos pos) throw(IOException) {
  this->stream.seekp(pos);
  if (this->stream.fail()) {
    THROW(IOException, "Could not reach specified seek position.");
  }
  if (this->stream.bad()) {
    THROW(IOException, "Error in seeking underlying ostream.");
  }
}

template<typename ostream_t>
void OStream<ostream_t>::seekp(streamoff off, ios_base::seekdir dir)
    throw(IOException) {
  this->stream.seekp(off, dir);
  if (this->stream.fail()) {
    THROW(IOException, "Could not reach specified seek position.");
  }
  if (this->stream.bad()) {
    THROW(IOException, "Error in seeking underlying ostream.");
  }
}

}
