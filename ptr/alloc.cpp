#include "ptr/alloc.h"

#ifdef PTR_MEMDEBUG
namespace ptr {
std::set<void*> __PTRS;
}
#endif
