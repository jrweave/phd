#ifndef __TEST__UNIT_H__
#define __TEST__UNIT_H__

#include <iostream>

#ifdef PTR_MEMDEBUG
#include "ptr/alloc.h"
#endif

#define INIT \
  unsigned int __ntests = 0; \
  unsigned int __failures = 0

#define TEST(call, ...) \
  std::cerr << "TEST " #call "(" #__VA_ARGS__ ")\n"; \
  if (!call(__VA_ARGS__)) __failures++; \
  __ntests++

#define PASS \
  std::cerr << "\nPASSED\n"; \
  return true

#define FAIL \
  std::cerr << std::dec << __LINE__ << " FAILED!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"; \
  return false

#define PROG(cond) \
  if (!(cond)) { FAIL; } \
  std::cerr << std::dec << __LINE__ << ","

#define MARK(msg) \
  std::cerr << "MARK " << __LINE__ << " " << msg << std::endl

#ifndef PTR_MEMDEBUG
#define FINAL \
  if (__failures > 0) std::cerr << std::dec << __failures << " FAILURES OUT OF "; \
  else std::cerr << "CLEARED ALL "; \
  std::cerr << std::dec << __ntests << " TESTS\n"; \
  exit(__failures)
#else
#define FINAL \
  if (ptr::__PTRS.size() > 0) { \
    std::set<void*>::iterator __it; \
    for (__it = ptr::__PTRS.begin(); __it != ptr::__PTRS.end(); ++__it) { \
      std::cerr << *__it << std::endl; \
    } \
    std::cerr << "THERE ARE " << ptr::__PTRS.size() << " HANGING POINTERS!\n"; \
    std::cerr << "+-------------------------------------+\n"; \
    std::cerr << "| XXXXX XXXXX X   X     XXXXX XXXXX X |\n"; \
    std::cerr << "| X       X    X X        X     X   X |\n"; \
    std::cerr << "| XXX     X     X         X     X   X |\n"; \
    std::cerr << "| X       X    X X        X     X     |\n"; \
    std::cerr << "| X     XXXXX X   X     XXXXX   X   X |\n"; \
    std::cerr << "+-------------------------------------+\n"; \
  } \
  if (__failures > 0) std::cerr << std::dec << __failures << " FAILURES OUT OF "; \
  else std::cerr << "CLEARED ALL "; \
  std::cerr << std::dec << __ntests << " TESTS\n"; \
  exit(__failures + ptr::__PTRS.size());
#endif

#endif /* __TEST__UNIT_H__ */
