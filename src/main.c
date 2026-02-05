/* psgame
 * Entry-point
 */

#include <stdio.h>
#include <sys/types.h>

#include <libetc.h>

#include "camera.h"
#include "common.h"

#include "cd.h"
#include "gfx.h"
#include "input.h"
#include "mem.h"
#include "text.h"

static Camera camera;
static Model cube;

static void _init(void);
static void _quit(void);

static void _update(void);

int main(void) {
	Font my_font;

	_init();

	text_init_font(&my_font, "\\FNT\\BIG;1", 10, 12);
	gfx_load_model(&cube, "\\MDL\\CUBE;1", "\\MDL\\CUBETEX;1");

	while( true ) {
		text_draw(&my_font, 32, 32, "Hello from PS1!");
		gfx_draw_model(&camera, &cube);
		gfx_display();
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

	VSyncCallback(_update);
}

static void _quit(void) {
	cd_quit();
	input_quit();
	gfx_quit();

	StopCallback();
}

static void _update(void) {
	cam_update(&camera);
}
