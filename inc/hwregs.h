#ifndef GUARD_PSGAME_HWREGS_H_
#define GUARD_PSGAME_HWREGS_H_

/* Adapted from the psyqo engine (MIT)
 *
 * Original source:
 *   https://github.com/grumpycoders/pcsx-redux/blob/main/src/mips/common/hardware/hwregs.h
 */

#define HW_U8(x) (*(volatile u_char *)(x))
#define HW_U16(x) (*(volatile u_short *)(x))
#define HW_U32(x) (*(volatile u_int *)(x))
#define HW_S8(x) (*(volatile char *)(x))
#define HW_S16(x) (*(volatile short *)(x))
#define HW_S32(x) (*(volatile int *)(x))

#define SBUS_DEV4_CTRL HW_U32(0x1f801014)
#define SBUS_DEV5_CTRL HW_U32(0x1f801018)
#define SBUS_COM_CTRL HW_U32(0x1f801020)

#define RAM_SIZE HW_U32(0x1f801060)

#define IREG HW_U32(0xbf801070)
#define IMASK HW_U32(0xbf801074)

#define DPCR HW_U32(0x1f8010f0)
#define DICR HW_U32(0x1f8010f4)

#define GPU_DATA HW_U32(0x1f801810)
#define GPU_STATUS HW_U32(0x1f801814)

#define ATCONS_STAT HW_U8(0x1f802000)
#define ATCONS_FIFO HW_U8(0x1f802002)
#define ATCONS_IRQ HW_U8(0x1f802030)
#define ATCONS_IRQ2 HW_U8(0x1f802032)

#define SYS573_WATCHDOG HW_U16(0x1f5c0000)
#define SYS573_7SEG_POST HW_U16(0x1f640010)

#define POST HW_U8(0xbf802041)

#endif // !GUARD_PSGAME_HWREGS_H_
