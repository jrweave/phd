#include "sys/endian.h"

namespace sys {

using namespace std;

bool is_big_endian() {
  static const union {
    uint32_t i;
    uint8_t c[4];
  } endint = { UINT32_C(0x01020304) };
  return endint.c[0] == 1;
}

bool is_little_endian() {
  return !is_big_endian();
}

}
