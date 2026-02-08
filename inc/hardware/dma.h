#ifndef GUARD_PSGAME_HARDWARE_DMA_H_
#define GUARD_PSGAME_HARDWARE_DMA_H_

/* Adapted from the psyqo engine (MIT)
 *
 * Original source:
 *   https://github.com/grumpycoders/pcsx-redux/blob/main/src/mips/common/hardware/dma.h
 */

#include <sys/types.h>

struct DMARegisters {
	void *MADR;
	u_int BCR, CHCR, padding;
};

#define DMA_CTRL ((volatile struct DMARegisters *)0x1f801080)

enum {
	DMA_MDECIN = 0,
	DMA_MDECOUT = 1,
	DMA_GPU = 2,
	DMA_CDROM = 3,
	DMA_SPU = 4,
	DMA_PIO = 5,
	DMA_GPUOTC = 6,
};

#endif // !GUARD_PSGAME_HARDWARE_DMA_H_
