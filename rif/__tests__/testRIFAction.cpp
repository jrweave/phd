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
#include "rif/RIFAction.h"

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

bool testState(RIFAction act, DPtr<uint8_t> *str, enum RIFActType type,
               bool is_ground) {
  PROG(act.getType() == type);
  DPtr<uint8_t> *s = act.toUTF8String();
  PROG(s->size() == str->size());
  PROG(memcmp(s->dptr(), str->dptr(), s->size()*sizeof(uint8_t)) == 0);
  s->drop();
  str->drop();
  PROG(act.isGround() == is_ground);
  PASS;
}

int main(int argc, char **argv) {
  INIT;
  TEST(testState, s2act("Assert( \"\"^^<s:>( ?arg1 \"function name\"^^<tag:jrweave@gmail.com,2012:/whocares>( ?v1 ?\"variable number 2\" ) ) )"), s2p("Assert(\"\"^^<s:>(?arg1 \"function name\"^^<tag:jrweave@gmail.com,2012:/whocares>(?v1 ?\"variable number 2\")))"), ASSERT_FACT, false);
  TEST(testState, s2act("Assert(\"object\"^^<s:>#\"class\"^^<s:>)"), s2p("Assert(\"object\"^^<s:> # \"class\"^^<s:>)"), ASSERT_FACT, true);
  TEST(testState, s2act("Assert(\"object\"^^<s:>[?attr->?val])"), s2p("Assert(\"object\"^^<s:>[?attr -> ?val])"), ASSERT_FACT, false);
  TEST(testState, s2act("Modify(\"object\"^^<s:>[?attr->?val])"), s2p("Modify(\"object\"^^<s:>[?attr -> ?val])"), MODIFY, false);
  TEST(testState, s2act("Execute( \"\"^^<s:>( ?arg1 \"function name\"^^<tag:jrweave@gmail.com,2012:/whocares>( ?v1 ?\"variable number 2\" ) ) )"), s2p("Execute(\"\"^^<s:>(?arg1 \"function name\"^^<tag:jrweave@gmail.com,2012:/whocares>(?v1 ?\"variable number 2\")))"), EXECUTE, false);
  TEST(testState, s2act("Retract( \"\"^^<s:>( ?arg1 \"function name\"^^<tag:jrweave@gmail.com,2012:/whocares>( ?v1 ?\"variable number 2\" ) ) )"), s2p("Retract(\"\"^^<s:>(?arg1 \"function name\"^^<tag:jrweave@gmail.com,2012:/whocares>(?v1 ?\"variable number 2\")))"), RETRACT_FACT, false);
  TEST(testState, s2act("Retract(\"object\"^^<s:>[?attr->?val])"), s2p("Retract(\"object\"^^<s:>[?attr -> ?val])"), RETRACT_FACT, false);
  TEST(testState, s2act("Retract(\"object\"^^<s:>)"), s2p("Retract(\"object\"^^<s:>)"), RETRACT_OBJECT, true);
  TEST(testState, s2act("Retract(\"object\"^^<s:> ?attr)"), s2p("Retract(\"object\"^^<s:> ?attr)"), RETRACT_SLOTS, false);
  TEST(testState, s2act("Execute(   )"), s2p("Execute()"), EXECUTE, true);
  FINAL;
}
