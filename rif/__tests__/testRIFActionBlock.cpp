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
#include "rif/RIFActionBlock.h"

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

RIFActionBlock p2ab(DPtr<uint8_t> *p) {
  RIFActionBlock ab = RIFActionBlock::parse(p);
  p->drop();
  return ab;
}

RIFActionBlock s2ab(const char *c) {
  return p2ab(s2p(c));
}

RIFAction p2act(DPtr<uint8_t> *p) {
  RIFAction a = RIFAction::parse(p);
  p->drop();
  return a;
}

RIFAction s2act(const char *c) {
  return p2act(s2p(c));
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

bool testState(RIFActionBlock ab, DPtr<uint8_t> *str, bool is_ground) {
  DPtr<uint8_t> *s = ab.toUTF8String();
  PROG(s->size() == str->size());
  PROG(memcmp(s->dptr(), str->dptr(), s->size()*sizeof(uint8_t)) == 0);
  s->drop();
  str->drop();
  PROG(ab.isGround() == is_ground);
  PASS;
}

int main(int argc, char **argv) {
  INIT;
  TEST(testState, s2ab("Do(Execute(   ))"), s2p("Do(Execute())"), true);
  TEST(testState, s2ab("Do(Execute( \"object name\"^^<some://arbitrary.url/>(?a ?b ?c)  ))"), s2p("Do(Execute(\"object name\"^^<some://arbitrary.url/>(?a ?b ?c)))"), false);
  TEST(testState, s2ab("Do((?a ?c[\"x\"^^<s:> -> ?a]) Retract( \"object name\"^^<some://arbitrary.url/>(?a ?b ?c)  ))"), s2p("Do((?a ?c[\"x\"^^<s:> -> ?a]) Retract(\"object name\"^^<some://arbitrary.url/>(?a ?b ?c)))"), false);
  TEST(testState, s2ab("Do((?a ?c[\"x\"^^<s:> -> ?a]) (?b New()) Retract( \"object name\"^^<some://arbitrary.url/>(?a ?b ?c)  ))"), s2p("Do((?a ?c[\"x\"^^<s:> -> ?a]) (?b New()) Retract(\"object name\"^^<some://arbitrary.url/>(?a ?b ?c)))"), false);
  TEST(testState, s2ab("Do((?a New()) Assert(?a # ?class) Execute())"), s2p("Do((?a New()) Assert(?a # ?class) Execute())"), false);
  FINAL;
}
