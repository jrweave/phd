/* Copyright 2012 Jesse Weaver
 * 
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 * 
 *        http://www.apache.org/licenses/LICENSE-2.0
 * 
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 *    implied. See the License for the specific language governing
 *    permissions and limitations under the License.
 */

#include "test/unit.h"
#include "rif/RIFRule.h"

using namespace ex;
using namespace iri;
using namespace ptr;
using namespace rif;
using namespace std;

DPtr<uint8_t> *s2p(const char *c) {
  DPtr<uint8_t> *p;
  NEW(p, MPtr<uint8_t>, strlen(c));
  ascii_strcpy(p->dptr(), c);
  return p;
}

RIFRule p2r(DPtr<uint8_t> *p) {
  RIFRule r = RIFRule::parse(p);
  p->drop();
  return r;
}

RIFRule s2r(const char *c) {
  return p2r(s2p(c));
}

RIFAtomic p2atom(DPtr<uint8_t> *p) {
  RIFAtomic a = RIFAtomic::parse(p);
  p->drop();
  return a;
}

RIFAtomic s2atom(const char *c) {
  return p2atom(s2p(c));
}

RIFConst p2c(DPtr<uint8_t> *p) {
  RIFConst c = RIFConst::parse(p);
  p->drop();
  return c;
}

RIFConst s2c(const char *c) {
  return p2c(s2p(c));
}

RIFTerm p2t(DPtr<uint8_t> *p) {
  RIFTerm t = RIFTerm::parse(p);
  p->drop();
  return t;
}

RIFTerm s2t(const char *c) {
  return p2t(s2p(c));
}

bool testState(RIFRule r, DPtr<uint8_t> *str, bool is_ground) {
  DPtr<uint8_t> *s = r.toUTF8String();
  cerr << s->size() << " =?= " << str->size() << endl;
  const uint8_t *c = s->dptr();
  const uint8_t *e = c + s->size();
  for (; c != e; ++c) {
    cerr << to_lchar(*c);
  }
  cerr << endl;
  PROG(s->size() == str->size());
  PROG(memcmp(s->dptr(), str->dptr(), s->size()*sizeof(uint8_t)) == 0);
  s->drop();
  str->drop();
  PROG(r.isGround() == is_ground);
  PASS;
}

int main(int argc, char **argv) {
  INIT;
  TEST(testState, s2r("If And(     )     Then       Do(   Execute( ) )"), s2p("If And() Then Do(Execute())"), true);

  TEST(testState, s2r("If And(?p[\"http://www.w3.org/2000/01/rdf-schema#subPropertyOf\"^^<http://www.w3.org/2007/rif#iri> -> ?q]\n"
                      "       ?s[?p -> ?o])\n"
                      "Then Do(Assert(?s[?q -> ?o]))"),
                  s2p("If And(?p[\"http://www.w3.org/2000/01/rdf-schema#subPropertyOf\"^^<http://www.w3.org/2007/rif#iri> -> ?q] ?s[?p -> ?o]) Then Do(Assert(?s[?q -> ?o]))"), false);

//  TEST(testState, s2ab("Do(Execute(   ))"), s2p("Do(Execute())"), true);
//  TEST(testState, s2ab("Do(Execute( \"object name\"^^<some://arbitrary.url/>(?a ?b ?c)  ))"), s2p("Do(Execute(\"object name\"^^<some://arbitrary.url/>(?a ?b ?c)))"), false);
//  TEST(testState, s2ab("Do((?a ?c[\"x\"^^<s:> -> ?a]) Retract( \"object name\"^^<some://arbitrary.url/>(?a ?b ?c)  ))"), s2p("Do((?a ?c[\"x\"^^<s:> -> ?a]) Retract(\"object name\"^^<some://arbitrary.url/>(?a ?b ?c)))"), false);
//  TEST(testState, s2ab("Do((?a ?c[\"x\"^^<s:> -> ?a]) (?b New()) Retract( \"object name\"^^<some://arbitrary.url/>(?a ?b ?c)  ))"), s2p("Do((?a ?c[\"x\"^^<s:> -> ?a]) (?b New()) Retract(\"object name\"^^<some://arbitrary.url/>(?a ?b ?c)))"), false);
//  TEST(testState, s2ab("Do((?a New()) Assert(?a # ?class) Execute())"), s2p("Do((?a New()) Assert(?a # ?class) Execute())"), false);
  FINAL;
}
