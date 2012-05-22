#ifndef __IO__OSTREAM_H__
#define __IO__OSTREAM_H__

#include "io/OutputStream.h"

namespace io {

template<typename ostream_t>
class OStream : public OutputStream {
protected:
  ostream_t stream;
  OStream() throw();
public:
  OStream(ostream_t &stream) throw();
  virtual ~OStream() throw(IOException);
  virtual void close() throw(IOException);
  virtual void flush() throw(IOException);
  virtual void write(DPtr<uint8_t> *buf, size_t &nwritten)
      throw(IOException, SizeUnknownException,
            BaseException<void*>);
  virtual streampos tellp() throw(IOException);
  virtual void seekp(streampos pos) throw(IOException);
  virtual void seekp(streamoff off, ios_base::seekdir dir) throw(IOException);
};

}

#include "OStream-inl.h"

#endif /* __IO__IOSTREAM_H__ */
