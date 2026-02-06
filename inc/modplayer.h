#ifndef GUARD_PSGAME_MODPLAYER_H_
#define GUARD_PSGAME_MODPLAYER_H_

#include <sys/types.h>

typedef struct _MODSampleData {
	char name[22];
	union {
		u_short length;
		u_char lenarr[2];
	};

	u_char fine_tune;
	u_char volume;

	u_short loop_point;
	u_short loop_length;
} MODSampleData;

typedef struct _MODFileFormat {
	char name[20];
	MODSampleData samples[31];

	u_char song_length;
	u_char padding;
	u_char pattern_table[128];
	u_char signature[4];
} MODFileFormat;

/* Loads the module and readies it for playback
 * Note that the pointer must be aligned to a 4-byte boundary
 *
 * Returns the number of bytes needed if relocation is desired
 */
u_int mod_load(MODFileFormat *module);

/* Loads the module and readies it for playback
 * Note that the pointer must be aligned to a 4-byte boundary
 *
 * This function is meant to load a module file alongside a .smp file
 *
 * Returns the number of bytes needed if relocation is desired
 */
u_int mod_load_with_smp(MODFileFormat *module, const u_char *sample_data);

/* Returns the number of channels in this module
 * Returns 0 if the module is invalid
 */
u_int mod_check_channel_count(MODFileFormat *module);

#endif // !GUARD_PSGAME_MODPLAYER_H_
