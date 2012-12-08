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

#ifndef __PAR__MPIPARTIALFILEINPUTSTREAM_H__
#define __PAR__MPIPARTIALFILEINPUTSTREAM_H__

#include "par/MPIFileInputStream.h"

namespace par {

class MPIPartialFileInputStream : public MPIFileInputStream {
private:
  MPI::Request req;
  MPI::Offset begin;
  MPI::Offset at;
  MPI::Offset end;
  MPI::Offset marker;
  DPtr<uint8_t> *asyncbuf;
  void initialize(const size_t page_size);
protected:
  virtual void fillBuffer() throw(IOException);
public:
  // setting argument /end/ to a negative number means read to end of file
  MPIPartialFileInputStream(const MPI::File &f, const size_t page_size,
                            const MPI::Offset begin, const MPI::Offset end)
      throw(BadAllocException, TraceableException);
  MPIPartialFileInputStream(const MPI::Intracomm &comm, const char* filename,
                            int amode, const MPI::Info &info,
                            const size_t page_size, const MPI::Offset begin,
                            MPI::Offset end)
      throw(IOException, BadAllocException, TraceableException);
  virtual ~MPIPartialFileInputStream() throw(IOException);
  virtual bool mark(const int64_t read_limit) throw(IOException);
  virtual bool markSupported() const throw();
  virtual void reset() throw(IOException);
};

}

#endif /* __PAR__MPIPARTIALFILEINPUTSTREAM_H__ */
