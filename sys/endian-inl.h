#include "sys/endian.h"

#include "sys/ints.h"

namespace sys {

using namespace std;

// TODO check for common BYTE_ORDER macros

inline
bool is_big_endian() {
  static const union {
    uint32_t i;
    uint8_t c[4];
  } endint = { UINT32_C(0x01020304) };
  return endint.c[0] == 1;
}

inline
bool is_little_endian() {
  return !is_big_endian();
}

}
