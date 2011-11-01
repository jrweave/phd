#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include <cstring>
#include "sys/ints.h"

namespace sys {

using namespace std;

bool is_big_endian();

bool is_little_endian();

template<typename T>
T &reverse_bytes(T &t) {
  const size_t tsize = sizeof(T) / sizeof(uint8_t);
  const size_t max = tsize >> 1;
  uint8_t *p = (uint8_t *) &t;
  size_t i;
  for (i = 0; i < max; i++) {
    size_t j = tsize - i - 1;
    const uint8_t temp = p[i];
    p[i] = p[j];
    p[j] = temp;
  }
  return t;
}

}

#endif /* __ENDIAN_H__ */
