#include "ptr/DPtr.h"

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

bool testInt() THROWS(BadAllocException) {
  int val = -382;
  int *ip = (int *)malloc(2*sizeof(int));
  *ip = val;
  ip[1] = val + 1;
  DPtr<int> *p = new DPtr<int>(ip);
  if ((int*)p->ptr() != ip) return false;
  if (p->dptr() != ip) return false;
  if (**p != val) return false;
  if ((*p)[1] != val + 1) return false;
  // p->func(); shouldn't compile
  int max = 10;
  int i;
  for (i = 0; i < max; i++) {
    p->hold();
  }
  for (i = 0; i < max; i++) {
    p->drop();
  }
  if (p->dptr() != ip) return false;
  int *ip2 = (int *)malloc(sizeof(int));
  ip2[0] = val + 2;
  *p = ip2;
  // ip is still a valid pointer
  free(ip);
  if (p->dptr() == ip) return false;
  if (p->dptr() != ip2) return false;
  if (**p != val + 2) return false;
  p->hold();
  p->drop();
  p->drop(); // this deletes p
  // p->drop(); // should create memory error 
  // ip2 is still a valid pointer
  free(ip2);
  return true;
}
TRACE(BadAllocException, "uncaught")

int main (int argc, char **argv) {
  ASSERT(testInt);
}
