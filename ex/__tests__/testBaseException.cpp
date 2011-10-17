#include <iostream>
#include "ex/BaseException.h"

using namespace std;
using namespace ex;

int main(int argc, char **argv) {
  try {
    unsigned int line = __LINE__;
    try {
      line = __LINE__; THROW(string("Message"));
      throw __LINE__;
    } catch (Exception &e) {
      if(string(e.getFileName()) != string(__FILE__)) {
        throw __LINE__;
      }
      if(e.getLineNumber() != line) {
        throw __LINE__;
      }
      if(e.getMessage() != string("Message")) {
        throw __LINE__;
      }
      if(e.hasCause()) {
        throw __LINE__;
      }
      try {
        e.getCause();
        throw __LINE__;
      } catch (Exception &e2) {
        // ignore
      }
      try {
        RETHROW(e, "rethrown");
        throw __LINE__;
      } catch (Exception &e) {
        cout << "[TEST] " << e.what() << endl;
      }
    }
  } catch (int line) {
    cerr << "Failed at line " << line << endl;
  }
}
