#ifndef GUARD_PSGAME_SPU_H_
#define GUARD_PSGAME_SPU_H_

/* Adapted from the psyqo engine (MIT)
 *
 * Original source:
 *   https://github.com/grumpycoders/pcsx-redux/blob/main/src/mips/common/hardware/spu.h
 */

#include <sys/types.h>

#include "hwregs.h"

struct SPUVoice {
	u_short volume_left;
	u_short volume_right;
	u_short sample_rate;
	u_short sample_start_addr;
	u_short ad;
	u_short sr;
	u_short current_volume;
	u_short sample_repeat_addr;
};

#define SPU_VOICES ((volatile struct SPUVoice *)0x1f801c00)

#define SPU_VOL_MAIN_LEFT HW_U16(0x1f801d80)
#define SPU_VOL_MAIN_RIGHT HW_U16(0x1f801d82)
#define SPU_REVERB_LEFT HW_U16(0x1f801d84)
#define SPU_REVERB_RIGHT HW_U16(0x1f801d86)
#define SPU_KEY_ON_LOW HW_U16(0x1f801d88)
#define SPU_KEY_ON_HIGH HW_U16(0x1f801d8a)
#define SPU_KEY_OFF_LOW HW_U16(0x1f801d8c)
#define SPU_KEY_OFF_HIGH HW_U16(0x1f801d8e)
#define SPU_PITCH_MOD_LOW HW_U16(0x1f801d90)
#define SPU_PITCH_MOD_HIGH HW_U16(0x1f801d92)
#define SPU_NOISE_EN_LOW HW_U16(0x1f801d94)
#define SPU_NOISE_EN_HIGH HW_U16(0x1f801d96)
#define SPU_REVERB_EN_LOW HW_U16(0x1f801d98)
#define SPU_REVERB_EN_HIGH HW_U16(0x1f801d9a)

#define SPU_RAM_DTA HW_U16(0x1f801da6)
#define SPU_CTRL HW_U16(0x1f801daa)
#define SPU_RAM_DTC HW_U16(0x1f801dac)
#define SPU_STATUS HW_U16(0x1f801dae)
#define SPU_VOL_CD_LEFT HW_U16(0x1f801db0)
#define SPU_VOL_CD_RIGHT HW_U16(0x1f801db2)
#define SPU_VOL_EXT_LEFT HW_U16(0x1f801db4)
#define SPU_VOL_EXT_RIGHT HW_U16(0x1f801db6)

extern u_int spu_master_volume;

/* Initializes the SPU */
void spu_init(void);

/* Uploads instruments to the SPU */
void spu_upload_instruments(u_int addr, const u_char *data, u_int size);

/* Mutes the SPU */
void spu_mute(void);

/* Unmutes the SPU */
void spu_unmute(void);

/* Key on */
void spu_key_on(u_int voice_bits);

/* Sets the volume of the voice with the given ID */
void spu_set_voice_volume(u_int id, u_int left, u_int right);

/* Sets the sample rate of the voice with the given ID */
void spu_set_voice_sample_rate(u_int id, u_short sample_rate);

/* Sets the sample start adress of the voice with the given ID */
void spu_set_voice_sample_start_addr(u_int id, u_short sample_start_addr);

/* Resest the voice on the SPU with the given ID */
void spu_reset_voice(u_int id);

/* Waits until the SPU is idle */
void spu_wait_idle(void);

#endif // !GUARD_PSGAME_SPU_H_
