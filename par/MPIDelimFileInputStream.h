#ifndef __PAR__MPIDELIMFILEINPUTSTREAM_H__
#define __PAR__MPIDELIMFILEINPUTSTREAM_H__

#include "par/MPIFileInputStream.h"

namespace par {

class MPIDelimFileInputStream : public MPIFileInputStream {
private:
  MPI::Request req;
  MPI::Offset begin;
  MPI::Offset at;
  MPI::Offset end;
  MPI::Offset marker;
  DPtr<uint8_t> *asyncbuf;
  const uint8_t delim;
  bool marked;
  void initialize(const MPI::Intracomm &comm, const size_t page_size);
protected:
  virtual void fillBuffer() throw(IOException);
public:
  MPIDelimFileInputStream(const MPI::Intracomm &comm, const MPI::File &f,
                          const size_t page_size, const uint8_t delimiter)
      throw(BadAllocException, TraceableException);
  MPIDelimFileInputStream(const MPI::Intracomm &comm, const char *filename,
                          int amode, const MPI::Info &info,
                          const size_t page_size, const uint8_t delimiter)
      throw(IOException, BadAllocException, TraceableException);
  virtual ~MPIDelimFileInputStream() throw(IOException);
  virtual bool mark(const int64_t read_limit) throw(IOException);
  virtual bool markSupported() const throw();
  virtual void reset() throw(IOException);
  virtual DPtr<uint8_t> *readDelimited() throw(IOException, BadAllocException);
};

}

#include "par/MPIDelimFileInputStream-inl.h"

#endif /* __PAR__MPIDELIMFILEINPUTSTREAM_H__ */
