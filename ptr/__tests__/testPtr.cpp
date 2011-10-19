#include "ptr/Ptr.h"

#include <iostream>

#define TEST(call, ...) \
  cerr << "TEST " #call "(" #__VA_ARGS__ ")\n"; \
  call(__VA_ARGS__)

#define PASS \
  cerr << "\nPASSED\n"; \
  return true

#define FAIL \
  cerr << " FAILED!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"; \
  return false

#define PROG(cond) \
  cerr << __LINE__; \
  if (!(cond)) { FAIL; } \
  cerr << ","

using namespace ptr;
using namespace std;

bool testOverall() THROWS(BadAllocException) {
  void *vp = malloc(1);
  Ptr *p = new Ptr(vp);
  PROG(p->ptr() == vp);
  int max = 10;
  int i;
  for (i = 0; i < max; i++) {
    p->hold();
  }
  for (i = 0; i < max; i++) {
    p->drop();
  }
  PROG(p->ptr() == vp);
  void *vp2 = malloc(1);
  *p = vp2;
  // vp is still a valid pointer
  free(vp);
  PROG(p->ptr() != vp);
  PROG(p->ptr() == vp2);
  p->hold();
  p->drop();
  p->drop(); // this deletes p
  // p->drop(); // should create memory error 
  // vp2 is still a valid pointer
  free(vp2);
  PASS;
}
TRACE(BadAllocException, "uncaught")

bool testConstructors() THROWS(BadAllocException) {
  PROG(true);
  void *vp = malloc(1);
  PROG(vp != NULL);
  Ptr *p = new Ptr(vp);
  PROG(true);
  Ptr p3 (*p);
  PROG(true);
  Ptr *p2 = new Ptr(&p3);
  PROG(p->ptr() == vp);
  PROG(p2->ptr() == vp);
  PROG(p3.ptr() == vp);
  p->hold();
  p2->drop(); // now p2 is invalid
  PROG(p->ptr() == vp);
  PROG(p3.ptr() == vp);
  p->drop();
  //p->drop(); // now p is invalid
  PROG(p3.ptr() == vp);
  void *vp2 = malloc(1);
  p = new Ptr(vp2);
  p3 = p;
  PROG(p->ptr() == vp2);
  PROG(p3.ptr() == vp2);
  p3 = vp;
  PROG(p3.ptr() == vp);
  p3 = *p;
  PROG(p3.ptr() == vp2);
  p->drop(); // now p is invalid
  PROG(p3.ptr() == vp2);
  // vp is still valid
  free(vp);
  // vp2 will remain valid unless freed
  free(vp2);
  PASS;
}
TRACE(BadAllocException, "uncaught")

int main (int argc, char **argv) {
  TEST(testOverall);
  TEST(testConstructors);
}
