#include "ptr/MPtr.h"

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
  MPtr<int> *p = new MPtr<int>(ip, 2);
  PROG((int*)p->ptr() == ip);
  PROG(p->dptr() == ip);
  PROG(**p == val);
  PROG((*p)[1] == val + 1);
  PROG(p->sizeKnown());
  PROG(p->size() == 2);
  PROG(p->sizeInBytes() == 2 * sizeof(int));
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
  p->drop(); // p is invalid
  //p->hold(); // memory error
  p = new MPtr<int>(1);
  **p = val;
  PROG((*p)[0] == val);
  p->hold();
  p->drop();
  p->drop(); // this deletes p
  //p->drop(); // should create memory error 
  PASS;
}
TRACE(BadAllocException, "uncaught")

bool testConstructors() THROWS(BadAllocException) {
  char *vp = (char *)malloc(sizeof(char));
  PROG(vp != NULL);
  MPtr<char> *p = new MPtr<char>(vp, 0);
  MPtr<char> p3 (*p);
  MPtr<char> *p2 = new MPtr<char>(&p3);
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
  p = new MPtr<char>(5832);
  PROG(p->dptr() != p3.dptr());
  p3 = p;
  PROG(p->dptr() == p3.dptr());
  (*p)['a'] = 'a';
  p3['z'] = 'z';
  PROG(p3['z'] == 'z');
  PROG((*p)['a'] == 'a');
  p->drop(); // now p is invalid
  PASS;
}
TRACE(BadAllocException, "uncaught")

int main (int argc, char **argv) {
  TEST(testInt);
  TEST(testConstructors);
}
