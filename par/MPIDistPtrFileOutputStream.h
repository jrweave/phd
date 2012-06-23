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
