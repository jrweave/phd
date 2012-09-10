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

#ifndef __PAR__MPIFILEOUTPUTSTREAM_H__
#define __PAR__MPIFILEOUTPUTSTREAM_H__

#include <mpi.h>
#include "io/OutputStream.h"
#include "sys/ints.h"

namespace par {

using namespace io;
using namespace ptr;

class MPIFileOutputStream : public OutputStream {
protected:
  MPI::File file;
  size_t length;
  DPtr<uint8_t> *buffer;
  bool no_splitting;
  virtual size_t writeBuffer() throw(IOException) = 0;
  // ^^^ take the first length bytes in buffer and write out
  // ^^^ return number of bytes immediately written
public:
  MPIFileOutputStream(const MPI::File &f, const size_t page_size,
      const bool no_splitting)
      throw(BadAllocException, TraceableException);
  MPIFileOutputStream(const MPI::Intracomm &comm, const char *filename,
                      int amode, const MPI::Info &info, const size_t page_size,
                      const bool no_splitting)
      throw(IOException, BadAllocException, TraceableException);
  virtual ~MPIFileOutputStream() throw(IOException);
  virtual void close() throw(IOException);
  virtual void flush() throw(IOException);
  virtual void write(DPtr<uint8_t> *buf)
      throw(IOException, SizeUnknownException, BaseException<void*>);
  virtual void write(DPtr<uint8_t> *buf, size_t &nwritten)
      throw(IOException, SizeUnknownException, BaseException<void*>);
};

}

#include "MPIFileOutputStream-inl.h"

#endif /* __PAR__MPIFILEOUTPUTSTREAM_H__ */
