/* psgame
 * Text
 */

#include "common.h"
#include "gfx.h"
#include "image.h"

#include <libgte.h>
#include <libgpu.h>

#include <sys/types.h>

#include "text.h"

#define MAX_TEXT_SIZE (256)

void text_init_font(Font *font, const char *FILENAME, u_char cw, u_char ch) {
	TIM_IMAGE tim;
	image_load(FILENAME, &tim);
	image_upload(&tim);

	const u_char TIM_MODE = 2 - (tim.mode & 0x3);

	font->tpage = getTPage(tim.mode & 0x3, 0, tim.prect->x, tim.prect->y);

	/* Does this TIM image have a CLUT? */
	if( tim.mode & 0x8 ) {
		font->clut = getClut(tim.crect->x, tim.crect->y);
	}

	font->w = tim.prect->w << TIM_MODE;
	font->h = tim.prect->h;

	font->u = (tim.prect->x & 0x3f) << TIM_MODE;
	font->v = tim.prect->y & 0xff;

	font->base_u = font->u;
	font->base_v = font->v;

	font->col.r = 128;
	font->col.g = 128;
	font->col.b = 128;

	font->cw = cw;
	font->ch = ch;

	font->cr = font->w / font->cw;
	font->cc = font->h / font->ch;
}

void text_draw(Font *font, u_short x, u_short y, const char *TEXT) {
	text_drawn(font, x, y, TEXT, MAX_TEXT_SIZE);
}

void text_drawn(Font *font, u_short x, u_short y, const char *TEXT, u_short n) {
	const u_short INITIAL_POS = x;

	u_short i;
	char c = 0;

	font->col.r = 128;
	font->col.g = 128;
	font->col.b = 128;

	for( i = 0; i < n; ++i ) {
		c = TEXT[i];
		if( c == '\0' ) {
			break;
		}

		/* Special command */
		if( c < 0 ) {
			switch( c ) {
			/* Change text color */
			case '\x8A':
				font->col.r = TEXT[++i];
				font->col.g = TEXT[++i];
				font->col.b = TEXT[++i];
				break;
			}

			continue;
		}

		switch( c ) {
		/* New line */
		case '\n':
			x = INITIAL_POS;
			y += font->ch;
			break;
		/* Whitespace */
		case ' ':
			x += font->cw;
			break;
		default:
			/* Offset to start of ASCII printable characters */
			c -= 33;

			font->u = font->cw * (c % font->cr);
			font->u += font->base_u;

			font->v = font->ch * (c / font->cr);
			font->v += font->base_v;

			gfx_set_font(font, x, y + 8);

			x += font->cw;
			break;
		}
	}

	gfx_set_tpage(font->tpage);
}
