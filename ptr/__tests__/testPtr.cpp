#include "ptr/Ptr.h"

#include <iostream>

#define ASSERT(call, ...) \
  if (call(__VA_ARGS__)) { \
    cerr << "PASSED "; \
  } else { \
    cerr << "FAILED "; \
  } \
  cerr << "" #call "(" #__VA_ARGS__ ")\n"

using namespace ptr;
using namespace std;

bool testOverall() THROWS(BadAllocException) {
  void *vp = malloc(1);
  Ptr *p = new Ptr(vp);
  if (p->ptr() != vp) return false;
  int max = 10;
  int i;
  for (i = 0; i < max; i++) {
    p->hold();
  }
  for (i = 0; i < max; i++) {
    p->drop();
  }
  if (p->ptr() != vp) return false;
  void *vp2 = malloc(1);
  *p = vp2;
  // vp is still a valid pointer
  free(vp);
  if (p->ptr() == vp) return false;
  if (p->ptr() != vp2) return false;
  p->hold();
  p->drop();
  p->drop(); // this deletes p
  // p->drop(); // should create memory error 
  // vp2 is still a valid pointer
  free(vp2);
  return true;
}
TRACE(BadAllocException, "uncaught")

int main (int argc, char **argv) {
  ASSERT(testOverall);
}
