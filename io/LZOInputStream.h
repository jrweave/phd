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

#ifndef __IO__LZOINPUTSTREAM_H__
#define __IO__LZOINPUTSTREAM_H__

#include <deque>
#include "ex/BaseException.h"
#include "io/InputStream.h"
#include "lzo/lzo1x.h"

namespace io {

using namespace ex;
using namespace ptr;
using namespace std;

class LZOInputStream : public InputStream {
private:
  InputStream *input_stream;
  deque<uint64_t> *index;
  DPtr<uint8_t> *buffer;
  size_t offset;
  size_t length;
  size_t max_block_size;
  uint64_t count;
  uint32_t flags;
  uint32_t checksum;
  bool header_read;
  void readHeader(const uint8_t first_flag_byte) throw(IOException);
  bool readu32(uint32_t &num) throw(IOException);
  bool readBlock(const uint32_t len) throw(IOException);
  bool readBlock(const uint32_t len, const uint32_t maxlen) throw(IOException);
  void readFooter() throw(IOException);
public:
  LZOInputStream(InputStream *is, deque<uint64_t> *index)
      throw(BaseException<void*>, TraceableException);
  virtual ~LZOInputStream() throw(IOException);
  virtual void close() throw(IOException);
  virtual DPtr<uint8_t> *read() throw(IOException, BadAllocException);
  virtual DPtr<uint8_t> *read(const int64_t amount)
      throw(IOException, BadAllocException);
  virtual void reset() throw(IOException);
};

}

#endif /* __IO__LZOINPUTSTREAM_H__ */
