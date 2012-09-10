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

#ifndef __PAR__MPIDISTPTRFILEOUTPUTSTREAM_H__
#define __PAR__MPIDISTPTRFILEOUTPUTSTREAM_H__

#include "par/MPIFileOutputStream.h"

namespace par {

class MPIDistPtrFileOutputStream : public MPIFileOutputStream {
private:
  MPI::Intracomm &comm;
  unsigned long bytes_written;
  unsigned long last_file_length;
  DPtr<uint8_t> *asyncbuf;
  bool started;
protected:
  virtual size_t writeBuffer() throw(IOException);
public:
  MPIDistPtrFileOutputStream(MPI::Intracomm &comm, const MPI::File &f,
      const size_t page_size, const bool no_splitting)
      throw(BadAllocException, TraceableException);
  MPIDistPtrFileOutputStream(MPI::Intracomm &comm, const char *filename,
      int amode, const MPI::Info &info, const size_t page_size,
      const bool no_splitting)
      throw(IOException, BadAllocException, TraceableException);
  virtual ~MPIDistPtrFileOutputStream() throw(IOException);
  virtual void close() throw(IOException);
};

}

#endif /* __PAR__MPIDISTPTRFILEOUTPUTSTREAM_H__ */
