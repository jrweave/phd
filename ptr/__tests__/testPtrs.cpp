#include "test/unit.h"

#include <iostream>
#include "ptr/APtr.h"
#include "ptr/DPtr.h"
#include "ptr/MPtr.h"
#include "ptr/OPtr.h"
#include "ptr/Ptr.h"

using namespace ptr;
using namespace std;

template<typename ptr_type>
bool testComprehensive(ptr_type *p, DPtr<ptr_type> *p2) {
  try {
    size_t size = p2->size();
    size_t offset = 1;
    size_t len = 1;
    size_t index = 1;
    DPtr<int> *p3 = p2->sub(offset);
    DPtr<int> *p4 = p3->sub(offset, len);
    PROG(p == p2->ptr());
    PROG(p + offset == p3->ptr());
    PROG(p + 2*offset == p4->ptr());
    PROG(p2->ptr() == p2->dptr());
    PROG(p3->ptr() == p3->dptr());
    PROG(p4->ptr() == p4->dptr());
    PROG(p2->sizeKnown());
    PROG(p3->sizeKnown());
    PROG(p4->sizeKnown());
    PROG(p2->size() == size);
    PROG(p3->size() == size - offset);
    PROG(p4->size() == len);
    PROG(**p2 == p[0]);
    PROG(**p3 == p[offset]);
    PROG(**p4 == p[2*offset]);
    PROG((*p2)[index] == p[index]);
    PROG((*p3)[index] == p[offset + index]);
    PROG((*p4)[index] == p[2*offset + index]);
    *p3 = p4;
    PROG(p3->ptr() == p4->ptr());
    PROG(p3->dptr() == p4->dptr());
    PROG(p3->sizeKnown() == p4->sizeKnown());
    PROG(p3->size() == p4->size());
    PROG(**p3 == **p4);
    PROG((*p3)[index] == (*p4)[index]);
    *p4 = p2;
    PROG(p2->ptr() == p4->ptr());
    PROG(p2->dptr() == p4->dptr());
    PROG(p2->sizeKnown() == p4->sizeKnown());
    PROG(p2->size() == p4->size());
    PROG(**p2 == **p4);
    PROG((*p2)[index] == (*p4)[index]);
    p3->drop();
    p4->drop();
    PASS;
  } catch (BadAllocException &e) {
    cerr << e.what();
    FAIL;
  }
}

int main(int argc, char **argv) {
  INIT;
  
  size_t size = 10;
  int *nums = (int *)calloc(size, sizeof(int));
  size_t i;
  for (i = 0; i < size; i++) {
    nums[i] = i;
  }

  DPtr<int> *p = new DPtr<int>(nums, size);
  TEST(testComprehensive, nums, p);
  p->drop(); // won't delete nums because is DPtr

  p = new MPtr<int>(nums, size);
  TEST(testComprehensive, nums, p);
  p->drop(); // will delete nums because is MPtr

  // free(nums); // memory error

  nums = new int[size];
  for (i = 0; i < size; i++) {
    nums[i] = i;
  }

  p = new APtr<int>(nums, size);
  TEST(testComprehensive, nums, p);
  p->drop();

  // delete[] nums; // memory error

  DPtr<const int> *cnums = new MPtr<const int>(5);
  cnums->hold();
  (*cnums)[1];
  cnums->drop();
  cnums->drop();

  FINAL;
}
