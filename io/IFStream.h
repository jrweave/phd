#ifndef __IO__IFSTREAM_H__
#define __IO__IFSTREAM_H__

#include <fstream>
#include "io/IStream.h"

namespace io {

using namespace ex;
using namespace ptr;
using namespace std;

class IFStream : public IStream<ifstream> {
public:
  IFStream(const char *filename) throw(BadAllocException, IOException);
  IFStream(const char *filename, const size_t bufsize)
      throw(BadAllocException, IOException);
  virtual ~IFStream() throw(IOException);
  virtual void close() throw(IOException);
};

}

#include "IFStream-inl.h"

#endif /* __IO__IFSTREAM_H__ */
