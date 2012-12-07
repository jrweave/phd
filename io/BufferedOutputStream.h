/* Copyright 2012 Jesse Weaver
 * 
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 * 
 *        http://www.apache.org/licenses/LICENSE-2.0
 * 
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 *    implied. See the License for the specific language governing
 *    permissions and limitations under the License.
 */

#ifndef __IO__BUFFEREDOUTPUTSTREAM_H__
#define __IO__BUFFEREDOUTPUTSTREAM_H__

#include "io/OutputStream.h"

namespace io {

using namespace ex;
using namespace ptr;
using namespace std;

class BufferedOutputStream : public OutputStream {
private:
  DPtr<uint8_t> *buffer;
  OutputStream *output_stream;
  size_t length;
  bool allow_splitting;
protected:
  void writeBuffer() throw(IOException);
public:
  BufferedOutputStream(OutputStream *os, const size_t bufsize,
    const bool allow_splitting)
    throw(BaseException<void*>, BadAllocException);
  virtual ~BufferedOutputStream() throw(IOException);
  virtual void close() throw(IOException);
  virtual void flush() throw(IOException);
  virtual void write(DPtr<uint8_t> *buf, size_t &nwritten)
      throw(IOException, SizeUnknownException, BaseException<void*>);
};

}

#endif /* __IO__BUFFEREDOUTPUTSTREAM_H__ */
