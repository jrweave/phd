#include "ucs/UCSIter.h"

#include "sys/ints.h"

namespace ucs {

using namespace std;

UCSIter::UCSIter() {
  // do nothing
}

UCSIter::~UCSIter() {
  // do nothing
}

uint32_t UCSIter::next() {
  return this->advance()->current();
}

}
