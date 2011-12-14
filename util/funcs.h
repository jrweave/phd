#ifndef __UTIL__FUNCS_H__
#define __UTIL__FUNCS_H__

#include <cstddef>

namespace util {

using namespace std;

template<typename T>
T &reverse_bytes(T &t);

}

#include "util/funcs-inl.h"

#endif /* __UTIL__FUNCS_H__ */
