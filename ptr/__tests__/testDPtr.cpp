#include "ptr/DPtr.h"

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

bool testInt() THROWS(BadAllocException) {
  int val = -382;
  int *ip = (int *)malloc(2*sizeof(int));
  *ip = val;
  ip[1] = val + 1;
  DPtr<int> *p = new DPtr<int>(ip);
  PROG((int*)p->ptr() == ip);
  PROG(p->dptr() == ip);
  PROG(**p == val);
  PROG((*p)[1] == val + 1);
  PROG(!p->sizeKnown());
  // p->func(); shouldn't compile
  int max = 10;
  int i;
  for (i = 0; i < max; i++) {
    p->hold();
  }
  for (i = 0; i < max; i++) {
    p->drop();
  }
  PROG(p->dptr() == ip);
  int *ip2 = (int *)malloc(sizeof(int));
  ip2[0] = val + 2;
  *p = ip2;
  // ip is still a valid pointer
  free(ip);
  PROG(p->dptr() != ip);
  PROG(p->dptr() == ip2);
  PROG(**p == val + 2);
  p->hold();
  p->drop();
  p->drop(); // this deletes p
  // p->drop(); // should create memory error 
  // ip2 is still a valid pointer
  free(ip2);
  PASS;
}
TRACE(BadAllocException, "uncaught")

bool testConstructors() THROWS(BadAllocException) {
  char *vp = (char *)malloc(sizeof(char));
  PROG(vp != NULL);
  DPtr<char> *p = new DPtr<char>(vp);
  DPtr<char> p3 (*p);
  DPtr<char> *p2 = new DPtr<char>(&p3);
  PROG(p->ptr() == vp);
  PROG(p2->dptr() == vp);
  PROG(p3.dptr() == vp);
  p->hold();
  p2->drop(); // now p2 is invalid
  PROG(p->dptr() == vp);
  PROG(p3.dptr() == vp);
  p->drop();
  p->drop(); // now p is invalid
  PROG(p3.dptr() == vp);
  char *vp2 = (char *)malloc(11*sizeof(char));
  p = new DPtr<char>(vp2);
  p3 = p;
  PROG(p->dptr() == vp2);
  PROG(p3.dptr() == vp2);
  p3 = vp;
  PROG(p3.dptr() == vp);
  p3 = *p;
  PROG(p3.dptr() == vp2);
  *(p->dptr()) = 5;
  *(p->dptr()) += 6;
  PROG(**p == 11);
  PROG(*p3 == 11);
  p->drop(); // now p is invalid
  PROG(p3.dptr() == vp2);
  // vp is still valid
  free(vp);
  // vp2 will remain valid unless freed
  free(vp2);
  PASS;
}
TRACE(BadAllocException, "uncaught")

int main (int argc, char **argv) {
  TEST(testInt);
  TEST(testConstructors);
}
