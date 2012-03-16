#ifndef __PTR__ALLOC_H__
#define __PTR__ALLOC_H__

#include <cstdlib>

#define WHOLE(...) __VA_ARGS__

#ifndef PTR_MEMPRINT
#define PTR_PRINTA(p)
#define PTR_PRINTD(p)
#else
#define PTR_PRINTA(p) std::cerr << "[PTR_MEMPRINT] " << __FILE__ << ":" << __LINE__ << ": Allocated " << (ptr::__persist_ptrs ? "PERSISTANT " : "") << #p << "=" << (void*)p << std::endl
#define PTR_PRINTD(p) std::cerr << "[PTR_MEMPRINT] " << __FILE__ << ":" << __LINE__ << ": Deallocated " << #p << "=" << (void*)p << std::endl
#endif

#ifndef PTR_MEMDEBUG
#define NEW(i, c, ...) i = new c(__VA_ARGS__)
#define DELETE(i) delete i
#define NEW_ARRAY(a, t, s) a = new t[s]
#define DELETE_ARRAY(a) delete[] a
#define PERSIST_PTRS(b)
#else
#include <iostream>
#include <set>
namespace ptr {
extern std::set<void*> __PTRS;
extern unsigned long __persist_ptrs;
}
#define NEW(i, c, ...) \
  i = new c(__VA_ARGS__); \
  PTR_PRINTA(i); \
  if (ptr::__persist_ptrs == 0 && !ptr::__PTRS.insert((void*)i).second) \
    std::cerr << "[PTR_MEMDEBUG] " << __FILE__ << ":" << __LINE__ << ": Unexpected allocation to " << #i << "=" << (void*) i << ", which means whatever was previously allocated to that address was not deallocated using alloc.h.\n\t" __FILE__ ":" << __LINE__ << ": " #i " = new " #c "(" #__VA_ARGS__ ");" << std::endl
#define DELETE(i) \
  PTR_PRINTD(i); \
  if (ptr::__PTRS.erase((void*)i) != 1 && ptr::__persist_ptrs == 0) \
    std::cerr << "[PTR_MEMDEBUG] " << __FILE__ << ":" << __LINE__ << ": Call to delete something at " << #i << "=" << (void*) i << " for which there is no record of allocation.\n\t" __FILE__ ":" << __LINE__ << ": delete " #i ";" << std::endl; \
  delete i
#define NEW_ARRAY(i, c, s) \
  i = new c[s]; \
  PTR_PRINTA(i); \
  if (ptr::__persist_ptrs == 0 && !ptr::__PTRS.insert((void*)i).second) \
    std::cerr << "[PTR_MEMDEBUG] " << __FILE__ << ":" << __LINE__ << ": Unexpected allocation to " << #i << "=" << (void*) i << ", which means whatever was previously allocated to that address was not deallocated using alloc.h.\n\t" __FILE__ ":" << __LINE__ << ": " #i " = new " #c "[" #s "];" << std::endl
#define DELETE_ARRAY(i) \
  PTR_PRINTD(i); \
  if (ptr::__PTRS.erase((void*)i) != 1 && ptr::__persist_ptrs == 0) \
    std::cerr << "[PTR_MEMDEBUG] " << __FILE__ << ":" << __LINE__ << ": Call to delete something at " << #i << "=" << (void*) i << " for which there is no record of allocation.\n\t" __FILE__ ":" << __LINE__ << ": delete[] " #i ";" << std::endl; \
  delete[] i
#define PERSIST_PTRS(b) \
  ptr::__persist_ptrs += (b ? 1 : -1)
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
