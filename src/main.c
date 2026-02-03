/* psgame
 * Entry-point
 */

#include <sys/types.h>

#include <libetc.h>

#include "common.h"

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
}

void _quit(void) {
	StopCallback();
}
