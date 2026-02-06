/* psgame
 * SPU
 */

#include <sys/types.h>

#include "hwregs.h"
#include "dma.h"

#include "spu.h"

u_int spu_master_volume = 16384;

void spu_init(void) {
	DPCR |= 0x000b0000;
	SPU_VOL_MAIN_LEFT = 0x3800;
	SPU_VOL_MAIN_RIGHT = 0x3800;
	SPU_CTRL = 0;
	SPU_KEY_ON_LOW = 0;
	SPU_KEY_ON_HIGH = 0;
	SPU_KEY_OFF_LOW = 0xffff;
	SPU_KEY_OFF_HIGH = 0xffff;
	SPU_RAM_DTC = 4;
	SPU_VOL_CD_LEFT = 0;
	SPU_VOL_CD_RIGHT = 0;
	SPU_PITCH_MOD_LOW = 0;
	SPU_PITCH_MOD_HIGH = 0;
	SPU_NOISE_EN_LOW = 0;
	SPU_NOISE_EN_HIGH = 0;
	SPU_REVERB_EN_LOW = 0;
	SPU_REVERB_EN_HIGH = 0;
	SPU_VOL_EXT_LEFT = 0;
	SPU_VOL_EXT_RIGHT = 0;
	SPU_CTRL = 0x8000;
}

void spu_upload_instruments(u_int addr, const u_char *data, u_int size) {
	u_int bcr = size >> 6;
	if( size & 0x3f ) {
		++bcr;
	}

	bcr <<= 16;
	bcr |= 0x10;

	SPU_RAM_DTA = addr >> 3;
	SPU_CTRL = (SPU_CTRL & ~0x0030) | 0x0020;
	while( (SPU_CTRL & 0x0030) != 0x0020 )
		;

	SBUS_DEV4_CTRL &= ~0x0f000000;

	DMA_CTRL[DMA_SPU].MADR = (void *)data;
	DMA_CTRL[DMA_SPU].BCR = bcr;
	DMA_CTRL[DMA_SPU].CHCR = 0x01000201;

	while( (DMA_CTRL[DMA_SPU].CHCR & 0x01000000) != 0 )
		;
}

void spu_mute(void) {
	SPU_REVERB_RIGHT = 0;
	SPU_REVERB_LEFT = 0;
	SPU_VOL_MAIN_RIGHT = 0;
	SPU_VOL_MAIN_LEFT = 0;
}

void spu_unmute(void) {
	SPU_CTRL = 0xc000;
}

void spu_set_voice_volume(u_int id, u_int left, u_int right) {
	SPU_VOICES[id].volume_left = (left * spu_master_volume) >> 16;
	SPU_VOICES[id].volume_right = (right * spu_master_volume) >> 16;
}

void spu_set_voice_sample_rate(u_int id, u_short sample_rate) {
	SPU_VOICES[id].sample_rate = sample_rate;
}

void spu_reset_voice(u_int id) {
	SPU_VOICES[id].volume_left = 0;
	SPU_VOICES[id].volume_right = 0;
	SPU_VOICES[id].sample_rate = 0;
	SPU_VOICES[id].sample_start_addr = 0;
	SPU_VOICES[id].ad = 0x000f;
	SPU_VOICES[id].sr = 0;
	SPU_VOICES[id].current_volume = 0;
	SPU_VOICES[id].sample_repeat_addr = 0;
}
