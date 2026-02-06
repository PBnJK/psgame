/* psgame
 * Entry-point
 */

#include <sys/types.h>

#include <libetc.h>

#include "common.h"

#include "cd.h"
#include "game.h"
#include "gfx.h"
#include "input.h"
#include "mem.h"
#include "player.h"
#include "text.h"

static Font my_font;
static Model ico;
static Player player;

static void _init(void);
static void _quit(void);

static void _display(void);
static void _update(void);

int main(void) {
	_init();

	ico.trans.vz = 200;

	while( true ) {
		_display();
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
