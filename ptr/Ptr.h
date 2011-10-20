#ifndef __PTR_H__
#define __PTR_H__

#include "ex/TraceableException.h"
#include "ptr/BadAllocException.h"

namespace ptr {

using namespace ex;
using namespace std;

/**
 * Ptr Conventions
 *
 * The term "codepart" refers to a logical, self-contained portion of
 * code such as a class or function.
 *
 * 1. The hold and drop methods are only to be used when handling a Ptr*.
 *    When a Ptr is passed by value or even by reference (using &), or
 *    when a Ptr is used as a local value (didn't use the new operator),
 *    hold and drop must not be called; the Ptr can be used in the same
 *    manner as any other type.  The only exception is when a Ptr is
 *    passed by reference, and then a codepart stores the address of the
 *    Ptr in a Ptr*.  Then cases 3 and 4 below apply.
 * 2. The codepart that instantiates the Ptr using new must never call
 *    the hold method.
 * 3. A codepart that handles and persists a Ptr* (e.g., stored as a
 *    member of a class or the value of a static variable in a function)
 *    that it did not instantiate must call the hold method upon
 *    receiving the Ptr*.
 * 4. A codepart that handles and persists a Ptr*, whether or not it is
 *    the codepart that instantiated it, must call the drop method exactly
 *    once when the Ptr will no longer be used.
 * 5. Aside from the aforementioned cases, the hold and drop methods may
 *    NEVER be called.
 * 6. Except for exceptional circumstances (e.g., perhaps cleaning up
 *    references when crashing), the delete operator may NEVER be used
 *    on a Ptr*.
 */
class Ptr {
private:
  unsigned int *global_refs;
  unsigned int local_refs;
protected:
  void *p; // subclasses, be careful
  virtual void destroy() throw();
  void destruct() throw();
public:
  Ptr() throw(BadAllocException);
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
