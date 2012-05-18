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
};

}

#include "par/MPIFileInputStream-inl.h"

#endif /* __PAR__MPIFILEINPUTSTREAM_H__ */
