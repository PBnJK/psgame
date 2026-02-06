/* psgame
 * Trigonometry
 */

/* Based on:
 *   https://www.coranac.com/2009/07/sines
 *   https://github.com/spicyjpeg/ps1-bare-metal/blob/main/src/08_spinningCube/trig.c
 */

#include "trig.h"

#define A (1 << 12)
#define B (19900)
#define C (3516)

int trig_isin(int x) {
	int c, y;

	c = x << (30 - ISIN_SHIFT);
	x -= 1 << ISIN_SHIFT;

	x <<= 31 - ISIN_SHIFT;
	x >>= 31 - ISIN_SHIFT;
	x *= x;
	x >>= 2 * ISIN_SHIFT - 14;

	y = B - (x * C >> 14);
	y = A - (x * y >> 16);

	return (c >= 0) ? y : (-y);
}

int trig_isin2(int x) {
	int c, y;

	c = x << (30 - ISIN2_SHIFT);
	x -= 1 << ISIN2_SHIFT;

	x <<= 31 - ISIN2_SHIFT;
	x >>= 31 - ISIN2_SHIFT;
	x *= x;
	x >>= 2 * ISIN2_SHIFT - 14;

	y = B - (x * C >> 14);
	y = A - (x * y >> 16);

	return (c >= 0) ? y : (-y);
}
