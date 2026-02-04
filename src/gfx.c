/* psgame
 * GFX routines
 */

#include <sys/types.h>

#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>

#include "common.h"

#include "gfx.h"

/* Location in memory that holds a special region-dependent character */
#define REGION_ADDR (*(char *)0xBFC7FF52)

#define OT_PRECISION (11)
#define OT_LENGTH (1 << OT_PRECISION)

#define PACKET_LENGTH (32768)

/* Debug background colors */
#define DEBUG_COLOR_R (32)
#define DEBUG_COLOR_G (128)
#define DEBUG_COLOR_B (32)

static DISPENV disp[2];
static DRAWENV draw[2];

static RECT VRAM = { 0, 0, 1024, 512 };
static RECT screen;

static const int SCR_WIDTH = 320;
static const int SCR_CENTER_WIDTH = SCR_WIDTH / 2;

static int SCR_HEIGHT = 0;
static int SCR_CENTER_HEIGHT = 0;

static u_long ot[2][OT_LENGTH] = { 0 };
static char primitive_buffer[2][PACKET_LENGTH] = { 0 };

static volatile u_char active_buffer = 0;
static char *next_primitive = primitive_buffer[0];

static SPRT *p_spr;

static DR_TPAGE *p_dr_tpage;
static DR_STP *p_dr_stp;

void gfx_init(void) {
	LOG("* INIT GFX\n");

	/* Europe (PAL) */
	if( REGION_ADDR == 'E' ) {
		LOG("- Detected PAL region\n");
		SetVideoMode(MODE_PAL);
		SCR_HEIGHT = 256;
	} else {
		LOG("- Detected NTSC region\n");
		SetVideoMode(MODE_NTSC);
		SCR_HEIGHT = 240;
	}

	SCR_CENTER_HEIGHT = SCR_HEIGHT / 2;

	/* Reset GFX */
	ResetGraph(0);
	SetGraphDebug(0);

	/* Clear VRAM */
	ClearImage(&VRAM, 0, 0, 0);

	/* Initialize display & draw environments
	 *
	 * The DRAWENVs are what is being drawn to the screen on the current frame
	 * The DISPENVs are where the next frame is being rendered to
	 *
	 * On the next frame, the two swap (by flipping the active_buffer variable
	 * from 0 to 1 or vice-versa,) so what was rendered to the DISPENV is now
	 * drawn to the screen as the DRAWENV
	 *
	 * The two buffers are literally, physically on top of each other in VRAM,
	 * which is pretty funny
	 */
	SetDefDispEnv(&disp[0], 0, 0, SCR_WIDTH, SCR_HEIGHT);
	SetDefDrawEnv(&draw[0], 0, SCR_HEIGHT, SCR_WIDTH, SCR_HEIGHT);
	SetDefDispEnv(&disp[1], 0, SCR_HEIGHT, SCR_WIDTH, SCR_HEIGHT);
	SetDefDrawEnv(&draw[1], 0, 0, SCR_WIDTH, SCR_HEIGHT);

	/* Setup DRAWENVs
	 *
	 * Setting isbg to 1 makes the debug background color be drawn to the screen
	 * Setting dtd to 1 enables dithering (looks awesome)
	 */
	setRGB0(&draw[0], DEBUG_COLOR_R, DEBUG_COLOR_G, DEBUG_COLOR_B);
	draw[0].isbg = 1;
	draw[0].dtd = 1;

	setRGB0(&draw[1], DEBUG_COLOR_R, DEBUG_COLOR_G, DEBUG_COLOR_B);
	draw[1].isbg = 1;
	draw[1].dtd = 1;

	PutDispEnv(&disp[active_buffer]);
	PutDrawEnv(&draw[active_buffer]);

	/* Define the screen dimensions */
	setRECT(&screen, 0, 0, SCR_WIDTH, SCR_HEIGHT);

	/* Setup ordering tables */
	next_primitive = primitive_buffer[active_buffer];
	ClearOTagR(ot[active_buffer], OT_LENGTH);

	/* Turn on rendering! */
	SetDispMask(1);
}

void gfx_quit(void) {
	ResetGraph(3);
}

void gfx_display(void) {
	DrawSync(0);
	VSync(0);

	PutDispEnv(&disp[active_buffer]);
	PutDrawEnv(&draw[active_buffer]);

	DrawOTag(ot[active_buffer] + OT_LENGTH - 1);

	active_buffer ^= 1;
	next_primitive = primitive_buffer[active_buffer];

	ClearOTagR(ot[active_buffer], OT_LENGTH);
}

void gfx_set_sprite(Sprite *spr) {
	p_spr = (SPRT *)next_primitive;
	setSprt(p_spr);

	setXY0(p_spr, spr->x, spr->y);
	setWH(p_spr, spr->w, spr->h);
	setUV0(p_spr, spr->u, spr->v);
	setRGB0(p_spr, spr->col.r, spr->col.g, spr->col.b);
	p_spr->clut = spr->clut;

	addPrim(&ot[active_buffer], p_spr);

	++p_spr;
	next_primitive = (char *)p_spr;
}

void gfx_set_font(Font *font, u_short x, u_short y) {
	p_spr = (SPRT *)next_primitive;
	setSprt(p_spr);

	setXY0(p_spr, x, y);
	setWH(p_spr, font->cw, font->ch);
	setUV0(p_spr, font->u, font->v);
	setRGB0(p_spr, font->col.r, font->col.g, font->col.b);
	p_spr->clut = font->clut;

	addPrim(&ot[active_buffer], p_spr);

	++p_spr;
	next_primitive = (char *)p_spr;
}

void gfx_set_tpage(u_short tpage) {
	p_dr_tpage = (DR_TPAGE *)next_primitive;
	setDrawTPage(p_dr_tpage, 0, 1, tpage);

	addPrim(&ot[active_buffer], p_dr_tpage);

	++p_dr_tpage;
	next_primitive = (char *)p_dr_tpage;
}

void gfx_set_stp(u_short stp) {
	p_dr_stp = (DR_STP *)next_primitive;
	setDrawStp(p_dr_stp, stp);

	addPrim(&ot[active_buffer], p_dr_stp);

	++p_dr_stp;
	next_primitive = (char *)p_dr_stp;
}
