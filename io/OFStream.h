#ifndef __IO__OFSTREAM_H__
#define __IO__OFSTREAM_H__

#include <fstream>
#include "io/OStream.h"

namespace io {

using namespace std;

class OFStream : public OStream<ofstream> {
public:
  OFStream(const char *filename) throw();
  virtual ~OFStream() throw(IOException);
  virtual void close() throw(IOException);
};

}

#endif /* __IO__OFSTREAM_H__ */
