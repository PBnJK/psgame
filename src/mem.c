/* psgame
 * Memory management
 */

#include <sys/types.h>

#include <libapi.h>
#include <malloc.h>

#include "common.h"

#include "mem.h"

#define STACK_SIZE 0x10000

#define BYTES_TO_KB ((X) >> 10)

extern char __sp;
extern char __heap_start;

void mem_init(void) {
	LOG("* INIT MEMORY\n");

	const u_long HEAP_SIZE = (&__sp - STACK_SIZE) - &__heap_start;

	EnterCriticalSection();
	InitHeap3((u_long *)&__heap_start, HEAP_SIZE);
	ExitCriticalSection();
}

void *mem_alloc(const size_t size) {
	void *block = malloc3(size);
	if( !block ) {
		LOG("*** ERROR: malloc3(%d) is null!\n", size);
		return 0;
	}

	return block;
}

void mem_free(void *block) {
	free3(block);
}
