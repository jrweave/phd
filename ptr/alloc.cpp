#include "ptr/alloc.h"

#ifdef PTR_MEMDEBUG
namespace ptr {
std::set<void*> __PTRS;
unsigned long __persist_ptrs = 0;
}
#endif
