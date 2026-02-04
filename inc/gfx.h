#ifndef GUARD_PSGAME_GFX_H_
#define GUARD_PSGAME_GFX_H_

#include <sys/types.h>

#include "sprite.h"
#include "text.h"

/* Initializes the GFX subsystem */
void gfx_init(void);

/* Quits the GFX subsystem */
void gfx_quit(void);

/* Blits the ordering tables to the screen */
void gfx_display(void);

/* Sets a Sprite for rendering */
void gfx_set_sprite(Sprite *spr);

/* Sets a Font for rendering */
void gfx_set_font(Font *font, u_short x, u_short y);

/* Sets a TPAGE for rendering */
void gfx_set_tpage(u_short tpage);

/* Sets an STP for rendering */
void gfx_set_stp(u_short stp);

#endif // !GUARD_PSGAME_GFX_H_
