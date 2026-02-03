#ifndef GUARD_PSGAME_SPRITE_H_
#define GUARD_PSGAME_SPRITE_H_

#include <sys/types.h>

#include <libgte.h>
#include <libgpu.h>

typedef struct {
	u_short tpage;
	u_short clut;
	u_short u, v;
	u_short x, y;
	u_short w, h;
	CVECTOR col;
} Sprite;

void spr_load_from_image(TIM_IMAGE *tim, Sprite *spr);
void spr_load_from_file(const char *FILENAME, Sprite *spr);

#endif // !GUARD_PSGAME_SPRITE_H_
