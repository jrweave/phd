#ifndef __PTR__ALLOC_H__
#define __PTR__ALLOC_H__

#include <cstdlib>

#ifndef PTR_MEMPRINT
#define PTR_PRINTA(p)
#define PTR_PRINTD(p)
#else
#define PTR_PRINTA(p) std::cerr << "[PTR_MEMPRINT] Allocated " << (void*)p << std::endl
#define PTR_PRINTD(p) std::cerr << "[PTR_MEMPRINT] Deallocated " << (void*)p << std::endl
#endif

#ifndef PTR_MEMDEBUG
#define ALLOC(p, ...) alloc(p, __VA_ARGS__)
#define RALLOC(p, ...) ralloc(p, __VA_ARGS__)
#define DALLOC(p, ...) dalloc(p)
#define NEW(i, c, ...) i = new c(__VA_ARGS__)
#define DELETE(i) delete i
#define NEW_ARRAY(a, t, s) a = new t[s]
#define DELETE_ARRAY(a) delete[] a
#else
#include <iostream>
#include <set>
namespace ptr {
extern std::set<void*> __PTRS;
}
#define NEW(i, c, ...) \
  i = new c(__VA_ARGS__); \
  PTR_PRINTA(i); \
  if (!ptr::__PTRS.insert((void*)i).second) \
    std::cerr << "[PTR_MEMDEBUG] Unexpected allocation to " << (void*) i << ", which means whatever was previously allocated to that address was not deallocated using alloc.h.\n\t" __FILE__ ":" << __LINE__ << ": " #i " = new " #c "(" #__VA_ARGS__ ");" << std::endl
#define DELETE(i) \
  PTR_PRINTD(i); \
  if (ptr::__PTRS.erase((void*)i) != 1) \
    std::cerr << "[PTR_MEMDEBUG] Call to delete something at " << (void*) i << " for which there is no record of allocation.\n\t" __FILE__ ":" << __LINE__ << ": delete " #i ";" << std::endl; \
  delete i
#define NEW_ARRAY(i, c, s) \
  i = new c[s]; \
  PTR_PRINTA(i); \
  if (!ptr::__PTRS.insert((void*)i).second) \
    std::cerr << "[PTR_MEMDEBUG] Unexpected allocation to " << (void*) i << ", which means whatever was previously allocated to that address was not deallocated using alloc.h.\n\t" __FILE__ ":" << __LINE__ << ": " #i " = new " #c "[" #s "];" << std::endl
#define DELETE_ARRAY(i) \
  PTR_PRINTD(i); \
  if (ptr::__PTRS.erase((void*)i) != 1) \
    std::cerr << "[PTR_MEMDEBUG] Call to delete something at " << (void*) i << " for which there is no record of allocation.\n\t" __FILE__ ":" << __LINE__ << ": delete[] " #i ";" << std::endl; \
  delete[] i
#endif

  

namespace ptr {

using namespace std;

template<typename ptr_type>
bool alloc(ptr_type *&p, size_t num) throw();

template<typename ptr_type>
bool ralloc(ptr_type *&p, size_t num) throw();

template<typename ptr_type>
void dalloc(ptr_type *&p) throw();

}

#include "alloc-inl.h"

#endif /* __PTR__ALLOC_H__ */
