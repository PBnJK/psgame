#ifndef GUARD_PSGAME_TEXT_H_
#define GUARD_PSGAME_TEXT_H_

#include <sys/types.h>

#include <libgte.h>

typedef struct _Font {
	u_short tpage;
	u_short clut;
	u_short u, v;
	u_short base_u, base_v;
	CVECTOR col;
	u_char w, h;
	u_char cw, ch; /* Character width/height */
	u_char cr, cc; /* Characters-per-row/column */
} Font;

/* Initializes a font */
void text_init_font(Font *font, const char *FILENAME, u_char cw, u_char ch);

/* Writes text to the screen */
void text_draw(Font *font, u_short x, u_short y, const char *TEXT);

/* Writes at most "n" characters of text to the screen */
void text_drawn(Font *font, u_short x, u_short y, const char *TEXT, u_short n);

#endif // !GUARD_PSGAME_TEXT_H_
