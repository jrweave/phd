#ifndef __UTIL__HASH_H__
#define __UTIL__HASH_H__

#include "sys/ints.h"

namespace util {

uint32_t hash_jenkins_one_at_a_time(const uint8_t *begin, const uint8_t *end)
    throw();

}

#include "util/hash-inl.h"

#endif /* __UTIL__HASH_H__ */
