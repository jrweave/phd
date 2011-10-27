#ifndef __BADALLOCEXCEPTION_H__
#define __BADALLOCEXCEPTION_H__

#include <cstring>
#include <new>
#include "ex/TraceableException.h"

#define RETHROW_BAD_ALLOC \
  JUST_RETHROW(ptr::BadAllocException, "(rethrow)") \
  catch(std::bad_alloc &ba) { \
    THROWX(ptr::BadAllocException); \
  }

namespace ptr {

using namespace ex;
using namespace std;

class BadAllocException : public TraceableException {
private:
  size_t size;
  bool size_known;
public:
  BadAllocException(const char *file, const unsigned int line) throw();
  BadAllocException(const char *file, const unsigned int line, size_t size)
      throw();
  virtual ~BadAllocException() throw();

  // Inherited Methods
  const char *what() const throw();
};

}

#endif /* __BADALLOCEXCEPTION_H__ */
