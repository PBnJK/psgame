/* psgame
 * modplayer
 *
 * This is an adaption of the modplayer from the psyqo project, which is itself
 * a reverse engineering of, quote...
 *
 *   "[...] the file MODPLAY.BIN, located in the zip file "Asm-Mod" from
 *    http://hitmen.c02.at/html/psx_tools.html, that has the CRC32 bb91769f"
 *
 * ...slightly tweaked to work in Psy-Q
 *
 * Original source:
 *   https://github.com/grumpycoders/pcsx-redux/blob/main/src/mips/modplayer/modplayer.c
 */

#include <sys/types.h>

#include "common.h"

#include "gfx.h"

#include "hardware/hwregs.h"
#include "hardware/spu.h"

#include "modplayer.h"

#define FX_ARPEGGIO (0)
#define FX_PORTAMENTO_UP (1)
#define FX_PORTAMENTO_DOWN (2)
#define FX_GLISSANDO (3)
#define FX_VIBRATO (4)
#define FX_VOLUME_SLIDE_GLISSANDO (5)
#define FX_VOLUME_SLIDE_VIBRATO (6)
#define FX_TREMOLO (7)
#define FX_SAMPLE_JUMP (9)
#define FX_VOLUME_SLIDE (0xa)
#define FX_ORDER_JUMP (0xb)
#define FX_SET_VOLUME (0xc)
#define FX_PATTERN_BREAK (0xd)
#define FX_EXTENDED (0xe)
#define FX_SET_SPEED (0xf)

typedef struct _SPUChannelData {
	u_short note;
	short period;
	u_short slide_to;
	u_char slide_speed;
	u_char volume;
	u_char sample_id;
	char vibrato;
	u_char fx[4];
	u_short sample_pos;
} SPUChannelData;

typedef struct _SPUInstrumentData {
	u_short base_address;
	u_char fine_tune;
	u_char volume;
} SPUInstrumentData;

static SPUChannelData _spu_channel_data[24];
static SPUInstrumentData _spu_instrument_data[31];

/* clang-format off */
static const u_char SINE_TABLE[32] = {
	0x00, 0x18, 0x31, 0x4a, 0x61,
	0x78, 0x8d, 0xa1, 0xb4, 0xc5,
	0xd4, 0xe0, 0xeb, 0xf4, 0xfa,
	0xfd, 0xff, 0xfd, 0xfa, 0xf4,
	0xeb, 0xe0, 0xd4, 0xc5, 0xb4,
	0xa1, 0x8d, 0x78, 0x61, 0x4a,
	0x31, 0x18,
};

/*  C    C#   D    D#   E    F    F#   G    G#   A    A#   B */
const u_short PERIOD_TABLE[36 * 16] = {
    856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,  /*  octave 1 tune 0 */
    428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,  /*  octave 2 tune 0 */
    214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113,  /*  octave 3 tune 0 */
    850, 802, 757, 715, 674, 637, 601, 567, 535, 505, 477, 450,  /*  octave 1 tune 1 */
    425, 401, 379, 357, 337, 318, 300, 284, 268, 253, 239, 225,  /*  octave 2 tune 1 */
    213, 201, 189, 179, 169, 159, 150, 142, 134, 126, 119, 113,  /*  octave 3 tune 1 */
    844, 796, 752, 709, 670, 632, 597, 563, 532, 502, 474, 447,  /*  octave 1 tune 2 */
    422, 398, 376, 355, 335, 316, 298, 282, 266, 251, 237, 224,  /*  octave 2 tune 2 */
    211, 199, 188, 177, 167, 158, 149, 141, 133, 125, 118, 112,  /*  octave 3 tune 2 */
    838, 791, 746, 704, 665, 628, 592, 559, 528, 498, 470, 444,  /*  octave 1 tune 3 */
    419, 395, 373, 352, 332, 314, 296, 280, 264, 249, 235, 222,  /*  octave 2 tune 3 */
    209, 198, 187, 176, 166, 157, 148, 140, 132, 125, 118, 111,  /*  octave 3 tune 3 */
    832, 785, 741, 699, 660, 623, 588, 555, 524, 495, 467, 441,  /*  octave 1 tune 4 */
    416, 392, 370, 350, 330, 312, 294, 278, 262, 247, 233, 220,  /*  octave 2 tune 4 */
    208, 196, 185, 175, 165, 156, 147, 139, 131, 124, 117, 110,  /*  octave 3 tune 4 */
    826, 779, 736, 694, 655, 619, 584, 551, 520, 491, 463, 437,  /*  octave 1 tune 5 */
    413, 390, 368, 347, 328, 309, 292, 276, 260, 245, 232, 219,  /*  octave 2 tune 5 */
    206, 195, 184, 174, 164, 155, 146, 138, 130, 123, 116, 109,  /*  octave 3 tune 5 */
    820, 774, 730, 689, 651, 614, 580, 547, 516, 487, 460, 434,  /*  octave 1 tune 6 */
    410, 387, 365, 345, 325, 307, 290, 274, 258, 244, 230, 217,  /*  octave 2 tune 6 */
    205, 193, 183, 172, 163, 154, 145, 137, 129, 122, 115, 109,  /*  octave 3 tune 6 */
    814, 768, 725, 684, 646, 610, 575, 543, 513, 484, 457, 431,  /*  octave 1 tune 7 */
    407, 384, 363, 342, 323, 305, 288, 272, 256, 242, 228, 216,  /*  octave 2 tune 7 */
    204, 192, 181, 171, 161, 152, 144, 136, 128, 121, 114, 108,  /*  octave 3 tune 7 */
    907, 856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480,  /*  octave 1 tune -8 */
    453, 428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240,  /*  octave 2 tune -8 */
    226, 214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120,  /*  octave 3 tune -8 */
    900, 850, 802, 757, 715, 675, 636, 601, 567, 535, 505, 477,  /*  octave 1 tune -7 */
    450, 425, 401, 379, 357, 337, 318, 300, 284, 268, 253, 238,  /*  octave 2 tune -7 */
    225, 212, 200, 189, 179, 169, 159, 150, 142, 134, 126, 119,  /*  octave 3 tune -7 */
    894, 844, 796, 752, 709, 670, 632, 597, 563, 532, 502, 474,  /*  octave 1 tune -6 */
    447, 422, 398, 376, 355, 335, 316, 298, 282, 266, 251, 237,  /*  octave 2 tune -6 */
    223, 211, 199, 188, 177, 167, 158, 149, 141, 133, 125, 118,  /*  octave 3 tune -6 */
    887, 838, 791, 746, 704, 665, 628, 592, 559, 528, 498, 470,  /*  octave 1 tune -5 */
    444, 419, 395, 373, 352, 332, 314, 296, 280, 264, 249, 235,  /*  octave 2 tune -5 */
    222, 209, 198, 187, 176, 166, 157, 148, 140, 132, 125, 118,  /*  octave 3 tune -5 */
    881, 832, 785, 741, 699, 660, 623, 588, 555, 524, 494, 467,  /*  octave 1 tune -4 */
    441, 416, 392, 370, 350, 330, 312, 294, 278, 262, 247, 233,  /*  octave 2 tune -4 */
    220, 208, 196, 185, 175, 165, 156, 147, 139, 131, 123, 117,  /*  octave 3 tune -4 */
    875, 826, 779, 736, 694, 655, 619, 584, 551, 520, 491, 463,  /*  octave 1 tune -3 */
    437, 413, 390, 368, 347, 328, 309, 292, 276, 260, 245, 232,  /*  octave 2 tune -3 */
    219, 206, 195, 184, 174, 164, 155, 146, 138, 130, 123, 116,  /*  octave 3 tune -3 */
    868, 820, 774, 730, 689, 651, 614, 580, 547, 516, 487, 460,  /*  octave 1 tune -2 */
    434, 410, 387, 365, 345, 325, 307, 290, 274, 258, 244, 230,  /*  octave 2 tune -2 */
    217, 205, 193, 183, 172, 163, 154, 145, 137, 129, 122, 115,  /*  octave 3 tune -2 */
    862, 814, 768, 725, 684, 646, 610, 575, 543, 513, 484, 457,  /*  octave 1 tune -1 */
    431, 407, 384, 363, 342, 323, 305, 288, 272, 256, 242, 228,  /*  octave 2 tune -1 */
    216, 203, 192, 181, 171, 161, 152, 144, 136, 128, 121, 114,  /*  octave 3 tune -1 */
};

/* clang-format on */

static const u_char *_module_data = 0;
static u_int _speed = 0;
static u_int _tick = 0;
static u_int _bpm = 0;

static const u_char *_row_pointer = 0;
static bool _change_row_next_tick = 0;
static u_int _next_row = 0;
static bool _change_order_next_tick = 0;
static u_int _next_order = 0;
static u_char _pattern_delay = 0;

static u_int _loop_start = 0;
static u_int _loop_count = 0;

static int _stereo = 0;

/* Loads a MOD file */
static u_int _load_internal(MODFileFormat *module, const u_char *sample_data);

/* Sets the BPM of the module */
static void _set_bpm(u_int bpm);

/* Processes effects */
static void _update_effect(void);

/* Handles the arpeggio effect */
static inline void _do_arp_effect(
	SPUChannelData *const data, u_int id, u_char fx23);

/* Handles the portamento up effect */
static inline void _do_portamento_up_effect(
	SPUChannelData *const data, u_int id, u_char fx23);

/* Handles the portamento down effect */
static inline void _do_portamento_down_effect(
	SPUChannelData *const data, u_int id, u_char fx23);

/* Handles the glissando effect */
static inline void _do_glissando_effect(SPUChannelData *const data, u_int id);

/* Handles the vibrato effect */
static inline void _do_vibrato_effect(SPUChannelData *const data, u_int id);

/* Handles the tremolo effect */
static inline void _do_tremolo_effect(SPUChannelData *const data, u_int id);

/* Handles the volume slide effect */
static inline void _do_volume_slide_effect(
	SPUChannelData *const data, u_int id, u_char fx23);

/* Processes rows */
static void _update_row(void);

/* Short-hand for setting the volume on an SPU voice */
static inline void _set_voice_volume(u_int id, u_int volume);

/* Short-hand for setting the sample rate on an SPU voice with a period */
static inline void _set_voice_sample_rate(u_int id, u_int period);

u_int mod_hblanks;

u_int mod_current_row = 0;
u_int mod_current_order = 0;
u_int mod_current_pattern = 0;

u_int mod_channels = 0;
u_int mod_song_length = 0;

u_int mod_load(MODFileFormat *module) {
	return _load_internal(module, 0);
}

u_int mod_load_with_smp(MODFileFormat *module, const u_char *sample_data) {
	_load_internal(module, sample_data ? sample_data : (const u_char *)-1);
	return mod_channels;
}

void mod_relocate(u_char *buffer) {
	u_int i, size;
	u_int max_pattern_id = 0;

	if( _module_data == buffer ) {
		return;
	}

	for( i = 0; i < 128; ++i ) {
		if( max_pattern_id < _module_data[i] ) {
			max_pattern_id = _module_data[i];
		}
	}

	size = 132 + mod_channels * 0x100 * (max_pattern_id + 1);
	const u_char *old_buffer = _module_data;

	if( buffer < old_buffer ) {
		for( i = 0; i < size; ++i ) {
			*buffer++ = *old_buffer++;
		}
	} else if( buffer > old_buffer ) {
		buffer += size;
		old_buffer += size;

		for( i = 0; i < size; ++i ) {
			*--buffer = *--old_buffer;
		}
	}

	_module_data = buffer;
}

void mod_poll(void) {
	u_char new_pattern_delay = _pattern_delay;

	if( ++_tick < _speed ) {
		_update_effect();
	} else {
		_tick = 0;
		if( new_pattern_delay-- == 0 ) {
			_update_row();
			new_pattern_delay = _pattern_delay;

			if( ++mod_current_row >= 64 || _change_row_next_tick ) {
				mod_current_row = 0;
				if( ++mod_current_order >= mod_song_length ) {
					mod_current_order = 0;
				}

				mod_current_pattern = _module_data[mod_current_order];
			}
		} else {
			_update_effect();
		}
	}

	_pattern_delay = new_pattern_delay;
}

void mod_play_note(u_int channel, u_int sample_id, u_int note, short volume) {
	u_short start_addr;
	int new_period;
	SPUChannelData *const data = &_spu_channel_data[channel];
	SPUInstrumentData *const instrument = &_spu_instrument_data[sample_id];

	if( volume < 0 ) {
		volume = 0;
	}

	if( volume > 63 ) {
		volume = 63;
	}

	data->sample_pos = 0;
	spu_set_voice_volume(channel, volume << 8, volume << 8);

	start_addr = (instrument->base_address << 4) + data->sample_pos;
	spu_set_voice_sample_start_addr(channel, start_addr);

	spu_wait_idle();
	spu_key_on(1 << channel);

	data->note = note = note + instrument->fine_tune * 36;

	new_period = data->period = PERIOD_TABLE[note];
	_set_voice_sample_rate(channel, new_period);
}

void mod_play_sfx(u_int channel, u_int sample_id, u_int note, short volume) {
	u_int prev_volume;

	prev_volume = spu_master_volume;
	spu_master_volume = 16384;

	mod_play_note(channel, sample_id, note, volume);
	spu_master_volume = prev_volume;
}

u_int mod_check_channel_count(MODFileFormat *module) {
	const u_char *MAGIC = module->signature;
	if( (MAGIC[0] == 'H') && (MAGIC[1] == 'I') && (MAGIC[2] == 'T') ) {
		return MAGIC[3] - '0';
	}

	if( (MAGIC[0] == 'H') && (MAGIC[1] == 'M') ) {
		return ((MAGIC[2] - '0') * 10) + MAGIC[3] - '0';
	}

	return 0;
}

void mod_silence(void) {
	u_int i;

	spu_init();
	for( i = 0; i < 24; ++i ) {
		spu_reset_voice(i);
	}
}

static u_int _load_internal(MODFileFormat *module, const u_char *sample_data) {
	MODSampleData *sample;
	u_int i, size;
	u_int current_spu_address = 0x1010;
	u_int max_pattern_id = 0;

	spu_init();
	mod_channels = mod_check_channel_count(module);

	if( mod_channels == 0 ) {
		return 0;
	}

	for( i = 0; i < 31; ++i ) {
		sample = &module->samples[i];
		_spu_instrument_data[i].base_address = current_spu_address >> 4;
		_spu_instrument_data[i].fine_tune = sample->fine_tune;
		_spu_instrument_data[i].volume = sample->volume;
		current_spu_address += sample->lenarr[0] * 0x100 + sample->lenarr[1];
	}

	mod_song_length = module->song_length;
	for( i = 0; i < 128; ++i ) {
		if( max_pattern_id < module->pattern_table[i] ) {
			max_pattern_id = module->pattern_table[i];
		}
	}

	_module_data = (const u_char *)&module->pattern_table[0];
	size = 132 + mod_channels * 0x100 * (max_pattern_id + 1);

	if( sample_data && (sample_data != (const u_char *)-1) ) {
		spu_upload_instruments(
			0x1010, sample_data, current_spu_address - 0x1010);
	} else if( sample_data == 0 ) {
		spu_upload_instruments(
			0x1010, _module_data + size, current_spu_address - 0x1010);
	}

	mod_current_order = 0;
	mod_current_pattern = module->pattern_table[0];
	mod_current_row = 0;

	_speed = 6;
	_tick = 6;
	_row_pointer
		= _module_data + 132 + mod_current_pattern * mod_channels * 0x100;

	for( i = 0; i < 24; ++i ) {
		spu_reset_voice(i);
	}

	_change_row_next_tick = 0;
	_change_order_next_tick = 0;

	_loop_start = 0;
	_loop_count = 0;

	_pattern_delay = 0;
	__builtin_memset(_spu_channel_data, 0, sizeof(_spu_channel_data));

	spu_unmute();

	_set_bpm(125);
	return size;
}

static void _set_bpm(u_int bpm) {
	u_int status, base;
	bool is_pal_console, is_pal;

	_bpm = bpm;

	status = GPU_STATUS;
	is_pal_console = REGION_ADDR == 'E';
	is_pal = (status & 0x00100000) != 0;

	/* PAL video? */
	if( is_pal ) {
		/* ... on PAL console? */
		if( is_pal_console ) {
			base = 36092; /* 312.5*125*50.000/50 or 314*125*49.761/50 */
		} else {
			base = 39422; /* 312.5*125*50.460/50 or 314*125*50.219/50 */
		}
	} else {
		/* ... on PAL console? */
		if( is_pal_console ) {
			base = 38977; /* 262.5*125*59.393/50 or 263*125*59.280/50 */
		} else {
			base = 39336; /* 262.5*125*59.940/50 or 263*125*59.826/50 */
		}
	}

	mod_hblanks = base / bpm;
}

static void _update_effect(void) {
	u_int channel;

	const u_char *row_pointer = _row_pointer;
	const u_int channels = mod_channels;

	for( channel = 0; channel < channels; ++channel ) {
		const u_char fx23 = row_pointer[3];
		const u_char fx1 = row_pointer[2] & 0x0f;
		const u_char fx2 = fx23 & 0x0f;

		SPUChannelData *const data = &_spu_channel_data[channel];

		switch( fx1 ) {
		case FX_ARPEGGIO:
			_do_arp_effect(data, channel, fx23);
			break;
		case FX_PORTAMENTO_UP:
			_do_portamento_up_effect(data, channel, fx23);
			break;
		case FX_PORTAMENTO_DOWN:
			_do_portamento_down_effect(data, channel, fx23);
			break;
		case FX_VOLUME_SLIDE_GLISSANDO:
			_do_volume_slide_effect(data, channel, fx23);
			/* fallthrough */
		case FX_GLISSANDO:
			_do_glissando_effect(data, channel);
			break;
		case FX_VOLUME_SLIDE_VIBRATO:
			_do_volume_slide_effect(data, channel, fx23);
			/* fallthrough */
		case FX_VIBRATO:
			_do_vibrato_effect(data, channel);
			break;
		case FX_TREMOLO:
			_do_tremolo_effect(data, channel);
			break;
		case FX_VOLUME_SLIDE:
			_do_volume_slide_effect(data, channel, fx23);
			break;
		case FX_EXTENDED:
			switch( fx23 >> 4 ) {
			case 9: /* Retrigger sample */
				if( (_tick % fx2) == 0 ) {
					spu_key_on(1 << channel);
				}
				break;
			case 12: /* Cut sample */
				if( _tick != fx2 ) {
					break;
				}

				data->volume = 0;
				spu_set_voice_volume(channel, 0, 0);
			}
			break;
		}

		row_pointer += 4;
	}
}

static inline void _do_arp_effect(
	SPUChannelData *const data, u_int id, u_char fx23) {
	u_char arp_tick;
	u_int new_period;

	const u_char fx2 = fx23 & 0x0f;
	const u_char fx3 = fx23 >> 4;

	if( fx23 == 0 ) {
		return;
	}

	arp_tick = _tick;
	switch( arp_tick % 3 ) {
	case 0:
		new_period = data->period;
		break;
	case 1:
		new_period = PERIOD_TABLE[data->note + fx3];
		break;
	case 2:
		new_period = PERIOD_TABLE[data->note + fx2];
		break;
	}

	_set_voice_sample_rate(id, new_period);
}

static inline void _do_portamento_up_effect(
	SPUChannelData *const data, u_int id, u_char fx23) {
	u_int new_period;

	new_period = data->period;
	new_period -= fx23;
	if( new_period < 108 ) {
		new_period = 108;
	}

	data->period = new_period;
	_set_voice_sample_rate(id, new_period);
}
static inline void _do_portamento_down_effect(
	SPUChannelData *const data, u_int id, u_char fx23) {
	u_int new_period;

	new_period = data->period;
	new_period += fx23;
	if( new_period > 907 ) {
		new_period = 907;
	}

	data->period = new_period;
	_set_voice_sample_rate(id, new_period);
}

static inline void _do_glissando_effect(SPUChannelData *const data, u_int id) {
	u_short slide_to;
	u_int new_period;

	slide_to = data->slide_to;
	new_period = data->period;
	if( new_period < slide_to ) {
		new_period += data->slide_speed;
		if( new_period > slide_to ) {
			new_period = slide_to;
		}
	} else if( new_period > slide_to ) {
		new_period -= data->slide_speed;
		if( new_period < slide_to ) {
			new_period = slide_to;
		}
	}

	data->period = new_period;
	_set_voice_sample_rate(id, new_period);
}

static inline void _do_vibrato_effect(SPUChannelData *const data, u_int id) {
	char new_value;
	u_int new_period, mutation;

	mutation = data->vibrato & 0x1f;
	switch( data->fx[3] & 3 ) {
	case 0: /* Sine (retrigger) */
	case 3: /* 3 should be random, but it's barely supported anyways... */
		mutation = SINE_TABLE[mutation];
		break;
	case 1: /* Sawtooth (retrigger) */
		if( data->vibrato < 0 ) {
			mutation *= -8;
			mutation += 0xff;
		} else {
			mutation *= 8;
		}
		break;
	case 2: /* Square (retrigger) */
		mutation = 0xff;
		break;
	}

	mutation *= data->fx[1] >> 4;
	mutation >>= 7;

	new_period = data->period;
	if( data->vibrato < 0 ) {
		new_period -= mutation;
	} else {
		new_period += mutation;
	}

	new_value = data->vibrato;
	new_value += data->fx[1] & 0x0f;
	if( new_value > 32 ) {
		new_value -= 64;
	}

	data->vibrato = new_value;
	_set_voice_sample_rate(id, new_period);
}

static inline void _do_tremolo_effect(SPUChannelData *const data, u_int id) {
	char new_value;
	short volume;
	u_int mutation;

	mutation = _spu_channel_data[0].fx[0] & 0x1f;
	switch( _spu_channel_data[0].fx[3] & 3 ) {
	case 0: /* Sine (retrigger) */
	case 3: /* 3 should be random, but it's barely supported anyways... */
		mutation = SINE_TABLE[mutation];
		break;
	case 1: /* Sawtooth (retrigger) */
		if( data->fx[0] & 0x80 ) {
			mutation *= -8;
			mutation += 0xff;
		} else {
			mutation *= 8;
		}
		break;
	case 2: /* Square (retrigger) */
		mutation = 0xff;
		break;
	}

	mutation *= data->fx[3] >> 4;
	mutation >>= 6;

	volume = data->volume;
	if( data->fx[0] & 0x80 ) {
		volume -= mutation;
	} else {
		volume += mutation;
	}

	if( volume > 63 ) {
		volume = 63;
	}

	new_value = data->fx[0] + (data->fx[2] & 0x0f);
	if( new_value >= 32 ) {
		new_value -= 64;
	}

	data->fx[0] = new_value;

	_set_voice_volume(id, volume);
}

static inline void _do_volume_slide_effect(
	SPUChannelData *const data, u_int id, u_char fx23) {
	short volume;

	const u_char fx3 = fx23 >> 4;

	volume = data->volume;
	if( fx23 <= 0x10 ) {
		volume -= fx23;
		if( volume < 0 ) {
			volume = 0;
		}
	} else {
		volume += fx3;
		if( volume > 63 ) {
			volume = 63;
		}
	}

	data->volume = volume;
	_set_voice_volume(id, volume);
}

static void _update_row(void) {
	u_int channel;
	const u_int channels = mod_channels;

	if( _change_order_next_tick ) {
		u_int new_order = _next_order;
		if( new_order >= mod_song_length ) {
			new_order = 0;
		}

		mod_current_row = 0;
		mod_current_order = new_order;
		mod_current_pattern = _module_data[new_order];
	}

	if( _change_row_next_tick ) {
		u_int new_row = (_next_row >> 4) * 10 + (_next_row & 0x0f);
		if( new_row >= 64 ) {
			new_row = 0;
		}

		mod_current_row = new_row;
		if( _change_order_next_tick ) {
			if( ++mod_current_order >= mod_song_length ) {
				mod_current_order = 0;
			}

			mod_current_pattern = _module_data[mod_current_order];
		}
	}

	_change_row_next_tick = false;
	_change_order_next_tick = false;

	_row_pointer = _module_data + 132 + mod_current_pattern * channels * 0x100
		+ mod_current_row * channels * 4;
	const u_char *row_pointer = _row_pointer;

	for( channel = 0; channel < channels; ++channel ) {
		short volume;
		u_char fx, fx0, fx1, fx2, fx3, fx23;
		u_int sample_id, period;
		int new_period;

		SPUChannelData *const data = &_spu_channel_data[channel];

		fx0 = row_pointer[0];
		fx1 = row_pointer[2];
		sample_id = (fx0 & 0xf0) | (fx1 >> 4);
		period = ((fx0 & 0x0f) << 8) | row_pointer[1];

		fx23 = row_pointer[3];
		fx2 = fx23 & 0x0f;
		fx3 = fx23 >> 4;

		fx1 &= 0x0f;
		if( fx1 != 9 ) {
			data->sample_pos = 0;
		}

		if( sample_id != 0 ) {
			u_short start_addr;
			SPUInstrumentData const *instrument
				= &_spu_instrument_data[sample_id];

			data->sample_id = --sample_id;

			volume = instrument->volume;
			if( volume > 63 ) {
				volume = 63;
			}

			data->volume = volume;
			if( fx1 != 7 ) {
				_set_voice_volume(channel, volume);
			}

			start_addr = (instrument->base_address << 4) + data->sample_pos;
			spu_set_voice_sample_start_addr(channel, start_addr);
		}

		if( period != 0 ) {
			int period_idx;
			for( period_idx = 35; period_idx > 0; --period_idx ) {
				if( PERIOD_TABLE[period_idx] == period ) {
					break;
				}
			}

			data->note = period_idx
				+ _spu_instrument_data[data->sample_id].fine_tune * 36;

			fx = data->fx[3];
			if( (fx & 0x0f) < 4 ) {
				data->vibrato = 0;
			}

			if( (fx >> 4) < 4 ) {
				data->fx[0] = 0;
			}

			if( (fx1 != 3) && (fx1 != 5) ) {
				spu_wait_idle();
				spu_key_on(1 << channel);
				data->period = PERIOD_TABLE[data->note];
			}

			new_period = data->period;
			_set_voice_sample_rate(channel, new_period);
		}

		switch( fx1 ) {
		case FX_GLISSANDO:
			if( fx23 != 0 ) {
				data->slide_speed = fx23;
			}
			if( period != 0 ) {
				data->slide_to = PERIOD_TABLE[data->note];
			}
			break;
		case FX_VIBRATO:
			if( fx3 != 0 ) {
				fx = data->fx[1];
				fx &= ~0x0f;
				fx |= fx3;
				data->fx[1] = fx;
			}
			if( fx2 != 0 ) {
				fx = data->fx[1];
				fx &= ~0xf0;
				fx |= fx3 << 4;
				data->fx[1] = fx;
			}
			break;
		case FX_TREMOLO:
			if( fx3 != 0 ) {
				fx = data->fx[2];
				fx &= ~0x0f;
				fx |= fx3;
				data->fx[2] = fx;
			}
			if( fx2 != 0 ) {
				fx = data->fx[2];
				fx &= ~0xf0;
				fx |= fx2 << 4;
				data->fx[2] = fx;
			}
			break;
		case FX_SAMPLE_JUMP:
			if( fx23 != 0 ) {
				data->sample_pos = fx23 << 7;
			}
			break;
		case FX_ORDER_JUMP:
			if( !_change_order_next_tick ) {
				_change_order_next_tick = 1;
				_next_order = fx23;
			}
			break;
		case FX_SET_VOLUME:
			volume = fx23;
			if( volume > 64 ) {
				volume = 63;
			}

			data->volume = volume;
			_set_voice_volume(channel, volume);
			break;
		case FX_PATTERN_BREAK:
			if( !_change_row_next_tick ) {
				_change_row_next_tick = true;
				_next_row = fx23;
			}
			break;
		case FX_EXTENDED:
			switch( fx3 ) {
			case 1: /* Fine slide up */
				new_period = data->period;
				new_period -= fx2;
				data->period = new_period;
				_set_voice_sample_rate(channel, new_period);
				break;
			case 2: /* Fine slide down */
				new_period = data->period;
				new_period += fx2;
				data->period = new_period;
				_set_voice_sample_rate(channel, new_period);
				break;
			case 4: /* Set vibrato waveform */
				fx = data->fx[3];
				fx &= ~0x0f;
				fx |= fx2;
				data->fx[3] = fx;
				break;
			case 5: /* Set finetune value */
				_spu_instrument_data[sample_id].fine_tune = fx2;
				break;
			case 6: /* Loop pattern */
				if( _loop_count-- == 0 ) {
					_loop_count = fx2;
				}
				if( _loop_count != 0 ) {
					mod_current_row = _loop_start;
				}
				break;
			case 7: /* Set tremolo waveform */
				fx = data->fx[3];
				fx &= ~0xf0;
				fx |= fx2 << 4;
				data->fx[3] = fx;
				break;
			case 10: /* Fine volume up */
				volume = data->volume;
				volume += fx2;
				if( volume > 63 ) {
					volume = 63;
				}

				data->volume = volume;
				_set_voice_volume(channel, volume);
				break;
			case 11: /* Fine volume down */
				volume = data->volume;
				volume -= fx2;
				if( volume < 0 ) {
					volume = 0;
				}

				data->volume = volume;
				_set_voice_volume(channel, volume);
				break;
			case 14: /* Delay pattern */
				_pattern_delay = fx2;
				break;
			}

			break;
		case FX_SET_SPEED:
			if( fx23 == 0 ) {
				break;
			}

			if( fx23 < 32 ) {
				_speed = fx23;
			} else {
				_set_bpm(fx23);
			}
			break;
		}

		row_pointer += 4;
	}
}

static inline void _set_voice_volume(u_int id, u_int volume) {
	volume <<= 8;
	if( _stereo ) {
		bool pan;
		short left, right;

		pan = (id & 1) ^ (id >> 1);
		left = pan ? 0 : volume;
		right = pan ? volume : 0;
		spu_set_voice_volume(id, left, right);
	}
}

static inline void _set_voice_sample_rate(u_int id, u_int period) {
	u_short sample_rate = ((7093789 / (period * 2)) << 12) / 44100;
	spu_set_voice_sample_rate(id, sample_rate);
}
