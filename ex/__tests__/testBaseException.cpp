#include "ex/BaseException.h"

#include <iostream>
#include "test/unit.h"

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
  INIT;
  TEST(testThrows);
  TEST(testWrap);
  FINAL;
}
