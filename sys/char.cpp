#include "sys/char.h"

#include "sys/sys.h"

#if SYSTEM == SYS_DEFAULT
// defined in char-inl.h
#else
#error Lacking definition of some char.h functions for specified system.
#endif
