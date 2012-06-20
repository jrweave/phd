#include "util/hash.h"

namespace util {

inline
uint32_t hash_jenkins_one_at_a_time(const uint8_t *begin, const uint8_t *end)
    throw() {
  uint32_t h = UINT32_C(0);
  for (; begin != end; ++begin) {
    h += *begin;
    h += (h << 10);
    h ^= (h >> 6);
  }
  h += (h << 3);
  h ^= (h >> 11);
  h += (h << 15);
  return h;
}

}
