/* psgame
 * Sprites
 */

#include <sys/types.h>

#include <libgte.h>
#include <libgpu.h>

#include "common.h"
#include "image.h"

#include "sprite.h"

void spr_load_from_image(TIM_IMAGE *tim, Sprite *spr) {
	spr->tpage = getTPage(tim->mode & 0x3, 0, tim->prect->x, tim->prect->y);

	/* Does this TIM image have a CLUT? */
	if( tim->mode & 0x8 ) {
		spr->clut = getClut(tim->crect->x, tim->crect->y);
	}

	spr->w = tim->prect->w << (2 - (tim->mode & 0x3));
	spr->h = tim->prect->h;

	spr->u = (tim->prect->x & 0x3f) << (2 - (tim->mode & 0x3));
	spr->v = tim->prect->y & 0xff;

	spr->col.r = 128;
	spr->col.g = 128;
	spr->col.b = 128;

	spr->x = 0;
	spr->y = 0;
}

void spr_load_from_file(const char *FILENAME, Sprite *spr) {
	TIM_IMAGE tim;

	image_load(FILENAME, &tim);
	image_upload(&tim);

	spr_load_from_image(&tim, spr);
}
