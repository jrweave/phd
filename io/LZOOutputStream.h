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

#ifndef __IO__LZOOUTPUTSTREAM_H__
#define __IO__LZOOUTPUTSTREAM_H__

#include <deque>
#include "io/OutputStream.h"
#include "lzo/lzo1x.h"
//#include "lzo/lzoconf.h"

namespace io {

using namespace ex;
using namespace ptr;
using namespace std;

class LZOOutputStream : public OutputStream {
private:
  OutputStream *output_stream;
  deque<uint64_t> *index;
  DPtr<uint8_t> *work_memory;
  DPtr<uint8_t> *output;
  uint64_t count;
  size_t max_block_size;
  uint32_t flags;
  uint32_t checksum;
  bool write_header;
  bool write_footer;
  bool header_written;
  void writeHeader() throw(IOException);
  void writeFooter() throw(IOException);
public:
  LZOOutputStream(OutputStream *os, deque<uint64_t> *index,
      const size_t max_block_size, const bool write_header,
      const bool write_footer, const bool do_checksum)
    throw(BaseException<void*>, BadAllocException, TraceableException);
  virtual ~LZOOutputStream() throw(IOException);
  virtual deque<uint64_t> *getIndex() throw();
  virtual void close() throw(IOException);
  virtual void flush() throw(IOException);
  virtual void write(DPtr<uint8_t> *buf, size_t &nwritten)
      throw(IOException, SizeUnknownException, BaseException<void*>);
};

}

#endif /* __IO__LZOOUTPUTSTREAM_H__ */
