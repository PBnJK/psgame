#ifndef GUARD_PSGAME_HARDWARE_COUNTERS_H_
#define GUARD_PSGAME_HARDWARE_COUNTERS_H_

/* Adapted from the psyqo engine (MIT)
 *
 * Original source:
 *   https://github.com/grumpycoders/pcsx-redux/blob/main/src/mips/common/hardware/counters.h
 */

#include <sys/types.h>

struct Counter {
	u_short value;
	u_short padding1;
	u_short mode;
	u_short padding2;
	u_short target;
	u_char padding[6];
};

#define COUNTERS ((volatile struct Counter *)0xbf801100)

#endif // !GUARD_PSGAME_HARDWARE_COUNTERS_H_
