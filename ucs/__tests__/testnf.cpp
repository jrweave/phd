#include "ucs/nf.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "ptr/APtr.h"
#include "ptr/DPtr.h"
#include "test/unit.h"
#include "sys/ints.h"

using namespace ptr;
using namespace std;

void printerr(const DPtr<uint32_t> *expected, const DPtr<uint32_t> *found) {
#if 1
  size_t i;
  cerr << "\nExpected: ";
  for (i = 0; i < expected->size(); i++) {
    cerr << hex << (*expected)[i] << " ";
  }
  cerr << endl << "Found: ";
  for (i = 0; i < found->size(); i++) {
    cerr << hex << (*found)[i] << " ";
  }
  cerr << endl;
#endif
}

bool testInvalid(DPtr<uint32_t> *codepoints) {
  #if defined(UCS_PLAY_DUMB) || defined(UCS_TRUST_CODEPOINTS)
  bool flip = true;
  #else
  bool flip = false;
  #endif
  size_t i;
  PROG(codepoints->sizeKnown());
  bool is_valid = true;
  for (i = 0; i < codepoints->size(); i++) {
    is_valid &= ucs::nfvalid((*codepoints)[i]);
  }
  PROG(flip ^ !is_valid);
  try {
    DPtr<uint32_t> *p = ucs::nfd(codepoints);
    p->drop();
    PROG(flip ^ false);
  } catch (ucs::InvalidCodepointException &e) {
    PROG(flip ^ true);
  }
  #ifndef UCS_NO_K
  try {
    DPtr<uint32_t> *p = ucs::nfkd(codepoints);
    p->drop();
    PROG(flip ^ false);
  } catch (ucs::InvalidCodepointException &e) {
    PROG(flip ^ true);
  }
  #endif
  #ifndef UCS_NO_C
  try {
    DPtr<uint32_t> *p = ucs::nfc(codepoints);
    p->drop();
    PROG(flip ^ false);
  } catch (ucs::InvalidCodepointException &e) {
    PROG(flip ^ true);
  }
  #ifndef UCS_NO_K
  try {
    DPtr<uint32_t> *p = ucs::nfkc(codepoints);
    p->drop();
    PROG(flip ^ false);
  } catch (ucs::InvalidCodepointException &e) {
    PROG(flip ^ true);
  }
  #endif
  #endif
  PASS;
}

bool test(const DPtr<uint32_t> *expected, const DPtr<uint32_t> *found) {
  size_t i;
  PROG(expected->sizeKnown());
  PROG(found->sizeKnown());
  printerr(expected, found);
  PROG(found->size() == expected->size());
  for (i = 0; i < found->size(); i++) {
    PROG((*expected)[i] == (*found)[i]);
  }
  PASS;
}

DPtr<uint32_t> *parse(const string &str) {
  vector<uint32_t> vec;
  stringstream ss (stringstream::in | stringstream::out);
  ss << str;
  uint32_t c;
  while (ss.good()) {
    ss >> hex >> c;
    vec.push_back(c);
  }
  APtr<uint32_t> *p = new APtr<uint32_t>(vec.size());
  copy(vec.begin(), vec.end(), p->dptr());
  return p;
}

int main(int argc, char **argv) {
  INIT;

  ifstream ifs ("../scripts/NormalizationTest.txt", ifstream::in);
  while (ifs.good()) {

    string line;
    getline(ifs, line);

    if (line.size() <= 0) {
      continue;
    }

    if (('A' > line[0] || line[0] > 'Z') &&
        ('0' > line[0] || line[0] > '9')) {
      continue;
    }

    stringstream ss (stringstream::in | stringstream::out);
    ss << line;
    string str, nfc, nfd, nfkc, nfkd;

    getline(ss, str, ';');
    getline(ss, nfc, ';');
    getline(ss, nfd, ';');
    getline(ss, nfkc, ';');
    getline(ss, nfkd, ';');
    
    DPtr<uint32_t> *input;
    DPtr<uint32_t> *expected;
    DPtr<uint32_t> *found;

    input = parse(str);

    cerr << "INPUT " << str << endl;
    cerr << "NFD " << nfd << endl;
#ifdef UCS_PLAY_DUMB
    expected = parse(str);
#else
    expected = parse(nfd);
#endif
    cerr << "parsed\n";
    found = ucs::nfd(input);
    cerr << "found?\n";
    TEST(test, expected, found);
    expected->drop();
    found->drop();

#ifndef UCS_NO_K
    cerr << "INPUT " << str << endl;
    cerr << "NFKD " << nfkd << endl;
#ifdef UCS_PLAY_DUMB
    expected = parse(str);
#else
    expected = parse(nfkd);
#endif
    found = ucs::nfkd(input);
    TEST(test, expected, found);
    expected->drop();
    found->drop();
#endif

#ifndef UCS_NO_C
    cerr << "INPUT " << str << endl;
    cerr << "NFC " << nfc << endl;
#ifdef UCS_PLAY_DUMB
    expected = parse(str);
#else
    expected = parse(nfc);
#endif
    found = ucs::nfc(input);
    TEST(test, expected, found);
    expected->drop();
    found->drop();

#ifndef UCS_NO_K
    cerr << "INPUT " << str << endl;
    cerr << "NFKC " << nfkc << endl;
#ifdef UCS_PLAY_DUMB
    expected = parse(str);
#else
    expected = parse(nfkc);
#endif
    found = ucs::nfkc(input);
    TEST(test, expected, found);
    expected->drop();
    found->drop();
#endif
#endif

    input->drop();

  }

  DPtr<uint32_t> *p = new APtr<uint32_t>(1);
  (*p)[0] = UINT32_C(0x0378);
  TEST(testInvalid, p);
  p->drop();

  FINAL;
}
