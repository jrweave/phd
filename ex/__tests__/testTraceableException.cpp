#include "ex/TraceableException.h"

#include "test/unit.h"

using namespace std;
using namespace ex;

void throws() THROWS(TraceableException) {
  THROW(TraceableException, "uh oh");
} TRACE(TraceableException, "unhandled")

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
  INIT;
  TEST(testThrows);
  FINAL;
}
