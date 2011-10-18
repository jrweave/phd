#include "ex/BaseException.h"

#include <iostream>

#define ASSERT(call, ...) \
  if (call(__VA_ARGS__)) { \
    cerr << "PASSED "; \
  } else { \
    cerr << "FAILED "; \
  } \
  cerr << "" #call "(" #__VA_ARGS__ ")\n"

using namespace std;
using namespace ex;

void throws() THROWS(BaseException<int>) {
  THROW(BaseException<int>, 5, "five");
} TRACE(BaseException<int>, "unhandled")

bool testThrows() {
  try {
    throws();
  } catch (BaseException<int> &e) {
    cout << e.what();
    return true;
  }
  return false;
}

bool testWrap() {
  try {
    throws();
  } catch (BaseException<int> &e) {
    try {
      THROW(BaseException<TraceableException*>, &e, "middle man");
    } catch (BaseException<TraceableException*> &e) {
      cout << e.what();
      return true;
    }
  }
  return false;
}

int main (int argc, char **argv) {
  ASSERT(testThrows);
  ASSERT(testWrap);
}
