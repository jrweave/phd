#include "ex/TraceableException.h"

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

void throws() THROWS(TraceableException) {
  THROW(TraceableException, "uh oh");
} TRACE("unhandled")

bool testThrows() {
  try {
    throws();
    return false;
  } catch (TraceableException &e) {
    cout << e.what();
    return true;
  }
}

int main (int argc, char **argv) {
  ASSERT(testThrows);
}
