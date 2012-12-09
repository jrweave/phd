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

#ifndef __PAR__MPIFILEINPUTSTREAM_H__
#define __PAR__MPIFILEINPUTSTREAM_H__

#include <mpi.h>
#include "io/InputStream.h"
#include "ptr/DPtr.h"

namespace par {

using namespace io;
using namespace ptr;
using namespace std;

class MPIFileInputStream : public InputStream {
protected:
  MPI::File file;
  size_t offset, length;
  DPtr<uint8_t> *buffer;
  virtual void fillBuffer() throw(IOException) = 0;
  // ^^^ modify buffer and set offset and length.
public:
  MPIFileInputStream(const MPI::File &f, const size_t page_size)
      throw(BadAllocException, TraceableException);
  MPIFileInputStream(const MPI::Intracomm &comm, const char* filename,
                     int amode, const MPI::Info &info, const size_t page_size)
      throw(IOException, BadAllocException, TraceableException);
  virtual ~MPIFileInputStream() throw(IOException);
  virtual int64_t available() throw(IOException);
  virtual void close() throw(IOException);
  virtual DPtr<uint8_t> *read() throw(IOException, BadAllocException);
  virtual DPtr<uint8_t> *read(const int64_t amount)
      throw(IOException, BadAllocException);
  virtual int64_t skip(const int64_t n) throw(IOException);
  MPI::Offset getFileSize() throw(IOException);
};

}

#include "par/MPIFileInputStream-inl.h"

#endif /* __PAR__MPIFILEINPUTSTREAM_H__ */
