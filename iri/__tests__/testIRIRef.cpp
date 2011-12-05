#include "iri/IRIRef.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include "iri/IRIRef.h"
#include "ptr/DPtr.h"
#include "ptr/MPtr.h"
#include "sys/ints.h"
#include "test/unit.h"

using namespace ex;
using namespace iri;
using namespace ptr;
using namespace std;

void print(DPtr<uint8_t> *utf8str) {
  size_t i;
  for (i = 0; i < utf8str->size(); i++) {
    cerr << (char) (*utf8str)[i];
  }
  cerr << " ; " << hex;
  for (i = 0; i < utf8str->size(); i++) {
    cerr << (int) (*utf8str)[i] << ' ';
  }
  cerr << endl << dec;
}

void print(IRIRef &iriref) {
  DPtr<uint8_t> *utf8str = iriref.getUTF8String();
  print(utf8str);
  utf8str->drop();
}

DPtr<uint8_t> *str2ptr(const char *str) {
  size_t len = strlen(str);
  DPtr<uint8_t> *ptr = new MPtr<uint8_t>(len);
  if (sizeof(char) == sizeof(uint8_t)) {
    memcpy(ptr->dptr(), str, len*sizeof(char));
  } else {
    copy(str, str + len, ptr->dptr());
  }
  return ptr;
}

IRIRef *ptr2iri(DPtr<uint8_t> *p) throw(MalformedIRIRefException) {
  IRIRef *iriref = NULL;
  try {
    iriref = new IRIRef(p);
  } catch (MalformedIRIRefException &e) {
    RETHROW(e, "(rethrow)");
  }
  return iriref;
}

IRIRef *str2iri(const char *str) throw(MalformedIRIRefException) {
  try {
    DPtr<uint8_t> *p = str2ptr(str);
    IRIRef *iriref = ptr2iri(p);
    return iriref;
  } catch (MalformedIRIRefException &e) {
    RETHROW(e, "(rethrow)");
  }
}

bool equiv(IRIRef *iriref, DPtr<uint8_t> *scheme, DPtr<uint8_t> *user_info,
    DPtr<uint8_t> *host, DPtr<uint8_t> *port, DPtr<uint8_t> *path,
    DPtr<uint8_t> *query, DPtr<uint8_t> *fragment) {
  if (scheme == NULL) {
    PROG(iriref->getPart(SCHEME) == NULL);
  } else {
    DPtr<uint8_t> *part = iriref->getPart(SCHEME);
    PROG(scheme->size() == part->size());
    PROG(equal(part->dptr(), part->dptr() + part->size(),
        scheme->dptr()));
    part->drop();
    scheme->drop();
  }
  if (user_info == NULL) {
    PROG(iriref->getPart(USER_INFO) == NULL);
  } else {
    DPtr<uint8_t> *part = iriref->getPart(USER_INFO);
    PROG(user_info->size() == part->size());
    PROG(equal(part->dptr(), part->dptr() + part->size(),
        user_info->dptr()));
    part->drop();
    user_info->drop();
  }
  if (host == NULL) {
    PROG(iriref->getPart(HOST) == NULL);
  } else {
    DPtr<uint8_t> *part = iriref->getPart(HOST);
    PROG(host->size() == part->size());
    PROG(equal(part->dptr(), part->dptr() + part->size(),
        host->dptr()));
    part->drop();
    host->drop();
  }
  if (port == NULL) {
    PROG(iriref->getPart(PORT) == NULL);
  } else {
    DPtr<uint8_t> *part = iriref->getPart(PORT);
    PROG(port->size() == part->size());
    PROG(equal(part->dptr(), part->dptr() + part->size(),
        port->dptr()));
    part->drop();
    port->drop();
  }
  if (path == NULL) {
    PROG(iriref->getPart(PATH) == NULL);
  } else {
    DPtr<uint8_t> *part = iriref->getPart(PATH);
    PROG(path->size() == part->size());
    PROG(equal(part->dptr(), part->dptr() + part->size(),
        path->dptr()));
    part->drop();
    path->drop();
  }
  if (query == NULL) {
    PROG(iriref->getPart(QUERY) == NULL);
  } else {
    DPtr<uint8_t> *part = iriref->getPart(QUERY);
    PROG(query->size() == part->size());
    PROG(equal(part->dptr(), part->dptr() + part->size(),
        query->dptr()));
    part->drop();
    query->drop();
  }
  if (fragment == NULL) {
    PROG(iriref->getPart(FRAGMENT) == NULL);
  } else {
    DPtr<uint8_t> *part = iriref->getPart(FRAGMENT);
    PROG(fragment->size() == part->size());
    PROG(equal(part->dptr(), part->dptr() + part->size(),
        fragment->dptr()));
    part->drop();
    fragment->drop();
  }
  delete iriref;
  PASS;
}

bool malformed(DPtr<uint8_t> *mal) {
  try {
    IRIRef *iriref = new IRIRef(mal);
  } catch (MalformedIRIRefException &e) {
    cerr << e.what() << endl;
    return true;
  }
  return false;
}

int main (int argc, char **argv) {
  INIT;
  TEST(equiv, str2iri("eXAMPLE://a/./b/../b/%63/%7bfoo%7d/ros%C3%A9"),
      str2ptr("eXAMPLE"), NULL, str2ptr("a"), NULL,
      str2ptr("/./b/../b/%63/%7bfoo%7d/ros%C3%A9"), NULL, NULL);
  TEST(equiv, str2iri("eXAMPLE://a/./b/../b/%63/%7bfoo%7d/ros%C3%A9")->resolve(NULL),
      str2ptr("eXAMPLE"), NULL, str2ptr("a"), NULL,
      str2ptr("/b/%63/%7bfoo%7d/ros%C3%A9"), NULL, NULL);
  TEST(equiv, str2iri("eXAMPLE://a/./b/../b/%63/%7bfoo%7d/ros%C3%A9")->normalize(),
      str2ptr("example"), NULL, str2ptr("a"), NULL,
      str2ptr("/b/c/%7Bfoo%7D/ros\xC3\xA9"), NULL, NULL);
  TEST(equiv, new IRIRef(), NULL, NULL, NULL, NULL, str2ptr(""), NULL, NULL);
  TEST(equiv, str2iri(""), NULL, NULL, NULL, NULL, str2ptr(""), NULL, NULL);
  TEST(equiv, str2iri("tag:jrweave@gmail.com,2011:#me"), str2ptr("tag"),
      NULL, NULL, NULL, str2ptr("jrweave@gmail.com,2011:"), NULL,
      str2ptr("me"));
  TEST(equiv, str2iri("s://@:/?#"), str2ptr("s"), str2ptr(""), str2ptr(""),
      str2ptr(""), str2ptr("/"), str2ptr(""), str2ptr(""));
  TEST(equiv, str2iri("s:"), str2ptr("s"), NULL, NULL, NULL, str2ptr(""),
      NULL, NULL);
  TEST(equiv, str2iri("//"), NULL, NULL, str2ptr(""), NULL, str2ptr(""),
      NULL, NULL);
  TEST(equiv, str2iri("/"), NULL, NULL, NULL, NULL, str2ptr("/"), NULL, NULL);
  TEST(equiv, str2iri("?"), NULL, NULL, NULL, NULL, str2ptr(""), str2ptr(""),
      NULL);
  TEST(equiv, str2iri("#"), NULL, NULL, NULL, NULL, str2ptr(""), NULL,
      str2ptr(""));
  TEST(malformed, str2ptr(":"));
  TEST(malformed, str2ptr("://@:/?#"));
  TEST(equiv,
    str2iri("ftp://user:password@host:21/url-path;type=typecode"),
    str2ptr("ftp"), str2ptr("user:password"), str2ptr("host"),
    str2ptr("21"), str2ptr("/url-path;type=typecode"), NULL, NULL);
  TEST(equiv, str2iri("http://192.168.0.1"), str2ptr("http"), NULL,
    str2ptr("192.168.0.1"), NULL, str2ptr(""), NULL, NULL);
  TEST(equiv, str2iri("http://[2001:0db8:85a3:0000:0000:8a2e:0370:7334]"),
    str2ptr("http"), NULL,
    str2ptr("[2001:0db8:85a3:0000:0000:8a2e:0370:7334]"), NULL, str2ptr(""),
    NULL, NULL);
  TEST(equiv, str2iri("http://[2001:db8:85a3:0:0:8a2e:370:7334]"),
    str2ptr("http"), NULL,
    str2ptr("[2001:db8:85a3:0:0:8a2e:370:7334]"), NULL, str2ptr(""),
    NULL, NULL);
  TEST(equiv, str2iri("http://[2001:db8:85a3::8a2e:370:7334]"),
    str2ptr("http"), NULL,
    str2ptr("[2001:db8:85a3::8a2e:370:7334]"), NULL, str2ptr(""),
    NULL, NULL);
  TEST(equiv, str2iri("http://[::]"),
    str2ptr("http"), NULL,
    str2ptr("[::]"), NULL, str2ptr(""),
    NULL, NULL);
  TEST(malformed, str2ptr("http://[:]"));
  TEST(malformed, str2ptr("http://[]"));
  TEST(equiv, str2iri("http://[::ffff:c000:280]"),
    str2ptr("http"), NULL,
    str2ptr("[::ffff:c000:280]"), NULL, str2ptr(""),
    NULL, NULL);
  TEST(equiv, str2iri("http://[::ffff:192.0.2.128]"),
    str2ptr("http"), NULL,
    str2ptr("[::ffff:192.0.2.128]"), NULL, str2ptr(""),
    NULL, NULL);
  FINAL;
}
