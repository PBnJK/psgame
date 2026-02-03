#ifndef GUARD_PSGAME_CD_H_
#define GUARD_PSGAME_CD_H_

#define CD_SECTOR_SIZE 2048
#define CALC_SECTOR_SIZE(LEN) (((LEN) + CD_SECTOR_SIZE - 1) / CD_SECTOR_SIZE)

#include <sys/types.h>

/* Initializes the CD subsystem */
void cd_init(void);

/* Quits the CD subsystem */
void cd_quit(void);

/* Loads a file from the CD using its filename */
u_long *cd_load_file_with_name(const char *FILENAME);

#endif // !GUARD_PSGAME_CD_H_
