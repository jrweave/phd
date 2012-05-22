#ifndef __IO__OUTPUTSTREAM_H__
#define __IO__OUTPUTSTREAM_H__

#include "ex/BaseException.h"
#include "io/IOException.h"
#include "ptr/DPtr.h"
#include "ptr/SizeUnknownException.h"
#include "sys/ints.h"

namespace io {

using namespace ex;
using namespace ptr;
using namespace std;

class OutputStream {
public:
  virtual ~OutputStream() throw(IOException);
  virtual void close() throw(IOException) = 0;
  virtual void flush() throw(IOException) = 0;
  virtual void write(DPtr<uint8_t> *buf)
      throw(IOException, SizeUnknownException,
            BaseException<void*>);
  virtual void write(DPtr<uint8_t> *buf, size_t &nwritten)
      throw(IOException, SizeUnknownException,
            BaseException<void*>) = 0;
};

}

#endif /* __IO__OUTPUTSTREAM_H__ */
