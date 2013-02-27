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

#include <limits>
#include "sys/ints.h"

typedef uint32_t sizeint_t; // S
#define SIZEINT_MAX std::numeric_limits<sizeint_t>::max()

typedef uint8_t varint_t; // V
#define VARINT_MAX std::numeric_limits<varint_t>::max()

typedef uint8_t atomtypeint_t; // AT
#define ATOMTYPEINT_MAX std::numeric_limits<atomtypeint_t>::max()

typedef uint64_t constint_t; // C
#define CONSTINT_MAX std::numeric_limits<constint_t>::max()

typedef uint8_t termtypeint_t; // TT
#define TERMTYPEINT_MAX std::numeric_limits<termtypeint_t>::max()

typedef uint8_t builtint_t; // B
#define BUILTINT_MAX std::numeric_limits<builtint_t>::max()

typedef uint8_t funcint_t; // F
#define FUNCINT_MAX std::numeric_limits<funcint_t>::max()

typedef uint8_t execint_t; // E
#define EXECINT_MAX std::numeric_limits<execint_t>::max()

typedef uint8_t acttypeint_t; // ACT
#define ACTTYPEINT_MAX std::numeric_limits<acttypeint_t>::max()

typedef uint8_t condtypeint_t; // CND
#define CONDTYPEINT_MAX std::numeric_limits<condtypeint_t>::max()

#define BUILTIN_PRED_LIST_CONTAINS ((builtint_t)0)

// THESE MUST HAVE HIGH-ORDER BIT SET TO 1
// THEY MUST START WITH 0x8000000000000001
// THEY MUST BE LISTED IN INCREMENTS OF 1
#define CONST_RDF_FIRST UINT64_C(0x8000000000000001)
#define CONST_RDF_REST UINT64_C(0x8000000000000002)
#define CONST_RDF_NIL UINT64_C(0x8000000000000003)
#define CONST_RIF_ERROR UINT64_C(0x8000000000000004)
