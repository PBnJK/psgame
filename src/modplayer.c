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
#include "dma.h"
#include "gfx.h"
#include "hwregs.h"
#include "spu.h"

#include "modplayer.h"

typedef struct _SPUChannelData {
	u_short note;
	short period;
	u_short slide_to;
	u_char slide_speed;
	u_char volume;
	u_char sampleID;
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

static u_int _channels = 0;
static u_int _song_length = 0;

static const u_char *_module_data = 0;
static u_int _current_order = 0;
static u_int _current_pattern = 0;
static u_int _current_row = 0;
static u_int _speed = 0;
static u_int _tick = 0;
static u_int _bpm = 0;

static const u_char *_row_pointer = 0;
static int _change_row_next_tick = 0;
static u_int _next_row = 0;
static int _change_order_next_tick = 0;
static u_int _next_order = 0;
static u_char _pattern_delay = 0;

static u_int _loop_start = 0;
static u_int _loop_count = 0;

static int _stereo = 0;
static u_int _hblanks;

/* Loads a MOD file */
static u_int _load_internal(MODFileFormat *module, const u_char *sample_data);

/* Sets the BPM of the module */
static void _set_bpm(u_int bpm);

u_int mod_load(MODFileFormat *module) {
	return _load_internal(module, 0);
}

u_int mod_load_with_smp(MODFileFormat *module, const u_char *sample_data) {
	_load_internal(module, sample_data ? sample_data : (const u_char *)-1);
	return _channels;
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

static u_int _load_internal(MODFileFormat *module, const u_char *sample_data) {
	MODSampleData *sample;
	u_int i;
	u_int current_spu_address = 0x1010;
	u_int max_pattern_id = 0;

	spu_init();
	_channels = mod_check_channel_count(module);

	if( _channels == 0 ) {
		return 0;
	}

	for( i = 0; i < 31; ++i ) {
		sample = &module->samples[i];
		_spu_instrument_data[i].base_address = current_spu_address >> 4;
		_spu_instrument_data[i].fine_tune = sample->fine_tune;
		_spu_instrument_data[i].volume = sample->volume;
		current_spu_address += sample->lenarr[0] * 0x100 + sample->lenarr[1];
	}

	_song_length = module->song_length;
	for( i = 0; i < 128; ++i ) {
		if( max_pattern_id < module->pattern_table[i] ) {
			max_pattern_id = module->pattern_table[i];
		}
	}

	_module_data = (const u_char *)&module->pattern_table[0];

	if( sample_data && (sample_data != (const u_char *)-1) ) {
		spu_upload_instruments(
			0x1010, sample_data, current_spu_address - 0x1010);
	} else if( sample_data == 0 ) {
		spu_upload_instruments(0x1010,
			_module_data + 132 + _channels * 0x100 * (max_pattern_id + 1),
			current_spu_address - 0x1010);
	}

	_current_order = 0;
	_current_pattern = module->pattern_table[0];
	_current_row = 0;

	_speed = 6;
	_tick = 6;
	_row_pointer = _module_data + 132 + _current_pattern * _channels * 0x100;

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
	return 132 + _channels * 0x100 * (max_pattern_id + 1);
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

	_hblanks = base / bpm;
}
