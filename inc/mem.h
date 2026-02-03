#ifndef GUARD_PSGAME_MEM_H_
#define GUARD_PSGAME_MEM_H_

#include <sys/types.h>

/* Initializes the memory subsystem */
void mem_init(void);

/* Allocates "size" bytes of data on RAM */
void *mem_alloc(const size_t size);

/* Frees a previously allocated block of memory */
void mem_free(void *block);

#endif // !GUARD_PSGAME_MEM_H_
