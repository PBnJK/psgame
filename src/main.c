/* psgame
 * Entry-point
 */

#include <sys/types.h>

#include <libetc.h>

#include "common.h"

#include "cd.h"
#include "input.h"
#include "mem.h"

void _init(void);
void _quit(void);

int main(void) {
	_init();

	while( true ) { }

	_quit();

	return 0;
}

void _init(void) {
	ResetCallback();

	cd_init();
	mem_init();
	input_init();
}

void _quit(void) {
	cd_quit();
	input_quit();

	StopCallback();
}
