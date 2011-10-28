#include "sys/endian.h"

#include "sys/sys.h"
#include "test/unit.h"

using namespace std;
using namespace sys;

bool testEndianess() {
#if SYSTEM == SYS_MACBOOK_PRO
  PROG(is_little_endian());
  PROG(!is_big_endian());
  PASS;
#else
#error "SYSTEM must be specified to a known value for this test.\n"
#endif
}

int main(int argc, char **argv) {
  INIT;
  TEST(testEndianess);
  FINAL;
}
