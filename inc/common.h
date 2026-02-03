#ifndef GUARD_CITYPEEP_COMMON_H_
#define GUARD_CITYPEEP_COMMON_H_

#include <sys/types.h>

#ifdef NDEBUG
#define LOG(...)
#else
#include <stdio.h>
#define LOG(...) printf(__FILE__, __LINE__, "%s:%d: " __VA_ARGS__)
#endif

typedef int bool;
#define false 0
#define true 1

/* Swap endianness (Little -> Big) */
#define LTOB32(x)                                                              \
	(((x) >> 24) | (((x) >> 8) & 0xFF00) | (((x) << 8) & 0x00FF0000)           \
		| ((x) << 24))

#endif // !GUARD_CITYPEEP_COMMON_H_
