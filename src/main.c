/* psgame
 * Entry-point
 */

#include <sys/types.h>

#include <libapi.h>
#include <libetc.h>

#include "common.h"

#include "cd.h"
#include "game.h"
#include "gfx.h"
#include "input.h"
#include "mem.h"
#include "modplayer.h"
#include "player.h"
#include "text.h"

#include "hardware/counters.h"
#include "hardware/irq.h"
#include "hardware/hwregs.h"

static Font my_font;
static Model ico;
static Player player;

static short _next_counter = 0;

static void _init(void);
static void _quit(void);

static void _display(void);
static void _update(void);

static void _check_music(void);
static void _wait_vsync(void);

int main(void) {
	_init();

	ico.trans.vz = 200;
	u_int row = 0xffffffff;
	u_int order = 0xffffffff;
	u_int pattern = 0xffffffff;

	while( true ) {
		_display();

		if( row != mod_current_row || order != mod_current_order
			|| pattern != mod_current_pattern ) {
			row = mod_current_row;
			order = mod_current_order;
			pattern = mod_current_pattern;
			printf("row: %02d / order: %02d / pattern: %02d\n", row, order,
				pattern);
		}
		_wait_vsync();
	}

	_quit();

	return 0;
}

static void _init(void) {
	ResetCallback();

	LOG("INITIALIZING EVERYTHING...\n");
	cd_init();
	mem_init();
	input_init();
	gfx_init();
	player_init(&player);

	VSyncCallback(_update);

	text_init_font(&my_font, "\\FNT\\BIG;1", 10, 12);
	gfx_load_model(&ico, "\\MDL\\ICO;1", 0);

	COUNTERS[1].mode = 0x0100;

	u_long *data = cd_load_file_with_name("\\AUD\\MOD\\TIMEWRPD;1");
	mod_load((MODFileFormat *)data);

	_next_counter = COUNTERS[1].value + mod_hblanks;

	LOG("%02d channels / %02d orders\n", mod_channels, mod_song_length);
}

static void _quit(void) {
	cd_quit();
	input_quit();
	gfx_quit();

	StopCallback();
}

static void _display(void) {
	player_display(&player);
	text_draw(&my_font, 0, 0, "DEMO");
	gfx_draw_model(&ico);
	gfx_display();
}

static void _update(void) {
	player_update(&player);
	game_update();
}

static void _check_music(void) {
	if( ((short)(_next_counter - COUNTERS[1].value)) <= 0 ) {
		mod_poll();
		_next_counter += mod_hblanks;
	}
}

static void _wait_vsync(void) {
	int was_locked = EnterCriticalSection();
	u_int imask = IMASK;

	IMASK = imask | IRQ_VBLANK;

	while( (IREG & IRQ_VBLANK) == 0 ) {
		_check_music();
	}

	IREG &= ~IRQ_VBLANK;
	IMASK = imask;

	if( !was_locked ) {
		ExitCriticalSection();
	}
}
