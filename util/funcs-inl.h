#include "util/funcs.h"

#include <cstring>
#include "sys/ints.h"

namespace util {

using namespace std;

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
