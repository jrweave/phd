#ifndef __UNIT_H__
#define __UNIT_H__

#include <iostream>

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

#define FINAL \
  if (__failures > 0) std::cerr << std::dec << __failures << " FAILURES OUT OF "; \
  else std::cerr << "CLEARED ALL "; \
  std::cerr << std::dec << __ntests << " TESTS\n"; \
  exit(__failures)

#endif /* __UNIT_H__ */
