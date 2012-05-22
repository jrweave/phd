#ifndef __IO__ISTREAM_H__
#define __IO__ISTREAM_H__

#include "ex/BaseException.h"
#include "ex/TraceableException.h"
#include "io/IOException.h"
#include "io/InputStream.h"
#include "ptr/DPtr.h"
#include "ptr/SizeUnknownException.h"
#include "sys/ints.h"

namespace io {

using namespace ex;
using namespace ptr;
using namespace std;

template<typename istream_t>
class IStream : public InputStream {
protected:
  istream_t stream;
  streampos marker;
  DPtr<uint8_t> *buffer;
  size_t offset, length;
  bool mark_support, marked;
  IStream(const size_t bufsize) throw(BadAllocException);
public:
  IStream(istream_t &stream) throw(BadAllocException);
  IStream(istream_t &stream, size_t bufsize)
      throw(BadAllocException, BaseException<size_t>);
  virtual ~IStream() throw(IOException);
  virtual int64_t available() throw(IOException);
  virtual void close() throw(IOException);
  virtual bool mark(const int64_t read_limit) throw(IOException);
  virtual bool markSupported() const throw();
  //virtual DPtr<uint8_t> *read() throw(IOException, BadAllocException);
  virtual DPtr<uint8_t> *read(const int64_t amount)
      throw(IOException, BadAllocException);
  virtual void reset() throw(IOException);
  virtual int64_t skip(const int64_t n) throw(IOException);
};

}

#include "IStream-inl.h"

#endif /* __IO__ISTREAM_H__ */
