#ifndef __PTR_H__
#define __PTR_H__

#include "ex/TraceableException.h"
#include "ptr/BadAllocException.h"

namespace ptr {

using namespace ex;
using namespace std;

class Ptr {
private:
  void *p;
  unsigned int *global_refs;
  unsigned int local_refs;
protected:
  virtual void destroy() throw();
public:
  Ptr(void *p) throw(BadAllocException);
  Ptr(const Ptr &ptr) throw();
  Ptr(const Ptr *ptr) throw();
  virtual ~Ptr() throw();

  // Final Methods
  void *ptr() const throw();
  void hold() throw();
  void drop() throw();

  // Operators
  Ptr &operator=(const Ptr &rhs) throw();
  Ptr &operator=(const Ptr *rhs) throw();
  Ptr &operator=(void *p) throw(BadAllocException);
};

}

#endif /* __PTR_H__ */
