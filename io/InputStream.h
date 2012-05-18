#ifndef __IO__INPUTSTREAM_H__
#define __IO__INPUTSTREAM_H__

#include "ex/BaseException.h"
#include "ex/TraceableException.h"
#include "io/IOException.h"
#include "ptr/DPtr.h"
#include "ptr/SizeUnknownException.h"
#include "sys/ints.h"

namespace io {

using namespace ex;
using namespace ptr;
using namespace std;

class InputStream {
public:
  virtual ~InputStream() throw(IOException);
  virtual int64_t available() throw(IOException);
  virtual void close() throw(IOException) = 0;
  virtual bool mark(const int64_t read_limit) throw(IOException);
  virtual bool markSupported() const throw();
  virtual DPtr<uint8_t> *read() throw(IOException, BadAllocException);
  virtual DPtr<uint8_t> *read(const int64_t amount)
      throw(IOException, BadAllocException) = 0;
  virtual void reset() throw(IOException);
  virtual int64_t skip(const int64_t n) throw(IOException);
};

}

#endif /* __IO__INPUTSTREAM_H__ */
