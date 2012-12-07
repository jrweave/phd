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

#ifndef __IO__BUFFEREDINPUTSTREAM_H__
#define __IO__BUFFEREDINPUTSTREAM_H__

#include "ex/BaseException.h"
#include "io/InputStream.h"

namespace io {

using namespace ex;
using namespace ptr;
using namespace std;

class BufferedInputStream : public InputStream {
private:
  DPtr<uint8_t> *buffer;
  InputStream *input_stream;
  size_t offset, length;
protected:
  virtual bool fillBuffer() throw(IOException);
public:
  BufferedInputStream(InputStream *is, const size_t buflen)
      throw(BaseException<void*>, BadAllocException);
  virtual ~BufferedInputStream() throw(IOException);
  virtual int64_t available() throw(IOException);
  virtual void close() throw(IOException);
  virtual DPtr<uint8_t> *read() throw(IOException, BadAllocException);
  virtual DPtr<uint8_t> *read(const int64_t amount)
      throw(IOException, BadAllocException);
  virtual void reset() throw(IOException);
};

}

#endif /* __IO__BUFFEREDINPUTSTREAM_H__ */
