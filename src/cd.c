/* psgame
 * CD
 */

#include <sys/types.h>

#include <libds.h>
#include <libetc.h>

#include "common.h"
#include "mem.h"

#include "cd.h"

void cd_init(void) {
	LOG("* INIT CD\n");

	if( !DsInit() ) {
		LOG("*** ERROR: couldn't initialize CD! ***\n");
		return;
	}

	DsSetDebug(0);
	DsControl(DslNop, 0, 0);
	DsStatus();

	DsControlB(DslSetmode, (u_char *)DslModeSpeed, 0);
	VSync(3);
}

void cd_quit(void) {
	DsClose();
}

u_long *cd_load_file_with_name(const char *FILENAME) {
	DslFILE f;
	u_long *buf = 0;

	if( !DsSearchFile(&f, (char *)FILENAME) ) {
		LOG("*** ERROR: file '%s' not found! ***\n", FILENAME);
		return 0;
	}

	const size_t SIZE_IN_SECTORS = CALC_SECTOR_SIZE(f.size);
	buf = mem_alloc(SIZE_IN_SECTORS * CD_SECTOR_SIZE);

	DsRead(&f.pos, SIZE_IN_SECTORS, buf, DslModeSpeed);
	while( DsReadSync(0) )
		;

	return buf;
}
