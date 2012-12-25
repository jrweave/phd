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
