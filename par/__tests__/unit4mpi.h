#ifndef __PAR__TESTS__UNIT4MPI_H__
#define __PAR__TESTS__UNIT4MPI_H__

#include <mpi.h>

#define INIT(argc, argv) \
  MPI::Init(argc, argv); \
  unsigned int __ntests = 0; \
  unsigned int __failures = 0

#define COMMRANK MPI::COMM_WORLD.Get_rank()

#define COMMSIZE MPI::COMM_WORLD.Get_size()

#define ONEBYONE_START \
  { \
    MPI::COMM_WORLD.Barrier(); \
    int rank = COMMRANK; \
    if (rank > 0) { \
      int i; \
      MPI::COMM_WORLD.Recv(&i, 1, MPI::INT, rank - 1, 999); \
    } \
  }

#define ONEBYONE_END \
  { \
    int rank = COMMRANK; \
    int size = COMMSIZE; \
    if (rank < size - 1) { \
      int i; \
      MPI::COMM_WORLD.Send(&i, 1, MPI::INT, rank + 1, 999); \
    } \
    MPI::COMM_WORLD.Barrier(); \
  }

#define TEST(call, ...) \
  if (COMMRANK == 0) { \
    std::cerr << "TEST " #call "(" #__VA_ARGS__ ")\n"; \
  } \
  MPI::COMM_WORLD.Barrier(); \
  if (!call(__VA_ARGS__)) __failures++; \
  __ntests++

#define PASS \
  ONEBYONE_START \
  std::cerr << "\n[" << COMMRANK << "] PASSED\n"; \
  ONEBYONE_END \
  return true

#define FAIL \
  ONEBYONE_START \
  std::cerr << std::dec << __LINE__ << " [" << COMMRANK << "] FAILED!\n"; \
  std::cerr << "+---------------------------+\n"; \
  std::cerr << "| XXXXX  XXX  XXXXX X     X |\n"; \
  std::cerr << "| X     X   X   X   X     X |\n"; \
  std::cerr << "| XXX   XXXXX   X   X     X |\n"; \
  std::cerr << "| X     X   X   X   X       |\n"; \
  std::cerr << "| X     X   X XXXXX XXXXX X |\n"; \
  std::cerr << "+---------------------------+\n"; \
  ONEBYONE_END \
  return false

#define PROG(cond) \
  if (!(cond)) { FAIL; } \
  ONEBYONE_START \
  if (COMMRANK == COMMSIZE - 1) { \
    std::cerr << std::dec << __LINE__ << ","; \
  } \
  ONEBYONE_END

#define MARK(msg) \
  ONEBYONE_START \
  if (COMMRANK == 0) { \
    std::cerr << "MARK " << __LINE__ << " " << msg; \
    std::cerr << " [checking in "; \
  } \
  if (COMMRANK == COMMSIZE - 1) { \
    std::cerr << COMMRANK << "]" << std::endl; \
  } else { \
    std::cerr << COMMRANK << ","; \
  } \
  ONEBYONE_END

#ifndef PTR_MEMDEBUG
#define FINAL \
  ONEBYONE_START \
  std::cerr << std::dec; \
  if (__failures > 0) { \
    std::cerr << "[" << COMMRANK << "] FAILED " << __failures << " OUT OF " << __ntests << " TESTS!\n"; \
    std::cerr << "+---------------------------+\n"; \
    std::cerr << "| XXXXX  XXX  XXXXX X     X |\n"; \
    std::cerr << "| X     X   X   X   X     X |\n"; \
    std::cerr << "| XXX   XXXXX   X   X     X |\n"; \
    std::cerr << "| X     X   X   X   X       |\n"; \
    std::cerr << "| X     X   X XXXXX XXXXX X |\n"; \
    std::cerr << "+---------------------------+\n"; \
    ONEBYONE_END \
    MPI::COMM_WORLD.Abort(__failures); \
  } \
  std::cerr << "[" << COMMRANK << "] CLEARED ALL " << __ntests << " TESTS\n"; \
  ONEBYONE_END \
  MPI::Finalize();
#else
#define FINAL \
  ONEBYONE_START \
  std::cerr << std::dec; \
  if (__failures > 0) { \
    std::cerr << "[" << COMMRANK << "] FAILED " << __failures << " OUT OF " << __ntests << " TESTS!\n"; \
    std::cerr << "+---------------------------+\n"; \
    std::cerr << "| XXXXX  XXX  XXXXX X     X |\n"; \
    std::cerr << "| X     X   X   X   X     X |\n"; \
    std::cerr << "| XXX   XXXXX   X   X     X |\n"; \
    std::cerr << "| X     X   X   X   X       |\n"; \
    std::cerr << "| X     X   X XXXXX XXXXX X |\n"; \
    std::cerr << "+---------------------------+\n"; \
    ONEBYONE_END \
    MPI::COMM_WORLD.Abort(__failures); \
  } \
  if (ptr::__PTRS.size() > 0) { \
    std::set<void*>::iterator __it; \
    for (__it = ptr::__PTRS.begin(); __it != ptr::__PTRS.end(); ++__it) { \
      std::cerr << *__it << std::endl; \
    } \
    std::cerr << "[" << COMMRANK << "] THERE ARE " << ptr::__PTRS.size() << " HANGING POINTERS!\n"; \
    std::cerr << "+-------------------------------------+\n"; \
    std::cerr << "| XXXXX XXXXX X   X     XXXXX XXXXX X |\n"; \
    std::cerr << "| X       X    X X        X     X   X |\n"; \
    std::cerr << "| XXX     X     X         X     X   X |\n"; \
    std::cerr << "| X       X    X X        X     X     |\n"; \
    std::cerr << "| X     XXXXX X   X     XXXXX   X   X |\n"; \
    std::cerr << "+-------------------------------------+\n"; \
    ONEBYONE_END \
    MPI::COMM_WORLD.Abort(ptr::__PTRS.size()); \
  } \
  std::cerr << "[" << COMMRANK << "] CLEARED ALL " << __ntests << " TESTS\n"; \
  ONEBYONE_END \
  MPI::Finalize()
#endif

#endif /* __PAR__TESTS__UNIT4MPI_H__ */
