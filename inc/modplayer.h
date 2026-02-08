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

extern u_int mod_hblanks;

extern u_int mod_current_row;
extern u_int mod_current_order;
extern u_int mod_current_pattern;

extern u_int mod_channels;
extern u_int mod_song_length;

/* Loads the module and readies it for playback*
 * Returns the number of bytes needed if relocation is desired
 *
 * NOTE: the pointer must be aligned to a 4-byte boundary
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

/* Relocates the module to a new buffer
 *
 * NOTE: caller is responsible for managing  memory
 */
void mod_relocate(u_char *buffer);

/* Call this function periodically to play sound */
void mod_poll(void);

/* Plays a single, arbitrary note
 *
 * channel is a channel ID between 0-23 (ideally above _channels)
 * sample_id is the ID of a sample in the MOD sample bank
 * note is a value between 0-35 (36 notes for 3 octaves)
 * volume is a value between 0-63
 */
void mod_play_note(u_int channel, u_int sample_id, u_int note, short volume);

/* Plays a sound effect
 * Unlike mod_play_note, the volume won't be affected by the global volume
 */
void mod_play_sfx(u_int channel, u_int sample_id, u_int note, short volume);

/* Returns the number of channels in this module
 * Returns 0 if the module is invalid
 */
u_int mod_check_channel_count(MODFileFormat *module);

/* Resets the SPU and silences everything */
void mod_silence(void);

#endif // !GUARD_PSGAME_MODPLAYER_H_
