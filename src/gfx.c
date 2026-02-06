/* psgame
 * GFX routines
 */

#include <sys/types.h>

#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>

#include <inline_n.h>
#include "cd.h"
#include "cpu_macros.h"

#include "common.h"
#include "image.h"
#include "game.h"
#include "mem.h"

#include "gfx.h"

/* Location in memory that holds a special region-dependent character */
#define REGION_ADDR (*(char *)0xBFC7FF52)

#define OT_PRECISION (11)
#define OT_LENGTH (1 << OT_PRECISION)

#define PACKET_LENGTH (32768)

/* Debug background colors */
#define DEBUG_COLOR_R (32)
#define DEBUG_COLOR_G (32)
#define DEBUG_COLOR_B (96)

/* Clip bitmask */
#define CLIP_LEFT (1)
#define CLIP_RIGHT (2)
#define CLIP_UP (4)
#define CLIP_DOWN (8)

/* Face types */
#define FACE_POLY_F3 (0)
#define FACE_POLY_FT3 (1)
#define FACE_POLY_G3 (2)
#define FACE_POLY_GT3 (3)

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

static MATRIX color_matrix = {
	{
		{ ONE * 3 / 4, 0, 0 }, /* Red */
		{ ONE * 3 / 4, 0, 0 }, /* Green */
		{ ONE * 3 / 4, 0, 0 }, /* Blue */
	},
	{ 0, 0, 0 },
};

static MATRIX light_matrix = {
	{
		{ -2048, -2048, -2048 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},
	{ 0, 0, 0 },
};

static int gte_result;
static int otz;

/* Primitives */
static POLY_F3 *p_poly_f3;
static POLY_FT3 *p_poly_ft3;
static POLY_G3 *p_poly_g3;
static POLY_GT3 *p_poly_gt3;

static SPRT *p_spr;

static DR_TPAGE *p_dr_tpage;
static DR_STP *p_dr_stp;

/* Prepares a mesh for drawing */
static inline int _prepare_mesh(Mesh *mesh, u_int i);

/* Tests if a vertex is within the bounds of the screen */
static int _test_clip(short x, short y);

/* Tests if a triangle should be clipped */
static int _test_tri_clip(DVECTOR *v0, DVECTOR *v1, DVECTOR *v2);

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

	/* Initialize the GTE */
	InitGeom();
	gte_SetGeomOffset(SCR_CENTER_WIDTH, SCR_CENTER_HEIGHT);
	gte_SetGeomScreen(SCR_CENTER_WIDTH);

	/* Fog setup */
	gte_SetBackColor(63, 63, 63);
	gte_SetColorMatrix(&color_matrix);

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

void gfx_init_model(Model *model) {
	setVector(&model->rot, 0, 0, 0);
	setVector(&model->trans, 0, 0, 0);
	setVector(&model->scale, ONE, ONE, ONE);
}

u_int gfx_load_model(Model *model, const char *FILENAME, const char *TEX) {
	u_long *data = cd_load_file_with_name(FILENAME);
	u_int size = gfx_load_model_from_ptr(model, data, TEX);

	mem_free(data);
	return size;
}

u_int gfx_load_model_from_ptr(Model *model, u_long *data, const char *TEX) {
	u_int i, j, tex_w;
	u_long *data_start = data;
	Mesh *mesh;

	LOG("* LOADING MODEL...\n");

	gfx_init_model(model);
	gfx_load_model_texture(model, TEX, &tex_w);

	model->mesh_count = *data++;

	for( i = 0; i < model->mesh_count; ++i ) {
		Face face;

		mesh = &model->meshes[i];

		mesh->vertex_count = *data;
		mesh->face_count = (*data++) >> 16;
		mesh->normal_count = *data;
		mesh->uv_count = (*data++) >> 16;

		/* Load vertices */
		for( j = 0; j < mesh->vertex_count; ++j ) {
			mesh->verts[j].vx = *data;
			mesh->verts[j].vy = (*data++) >> 16;
			mesh->verts[j].vz = *data;

			if( ++j < mesh->vertex_count ) {
				mesh->verts[j].vx = (*data++) >> 16;
				mesh->verts[j].vy = *data;
				mesh->verts[j].vz = (*data++) >> 16;
			}
		}

		if( mesh->vertex_count % 2 ) {
			++data;
		}

		/* Load faces */
		for( j = 0; j < mesh->face_count; ++j ) {
			mesh->faces[j].vx = *data;
			mesh->faces[j].vy = (*data++) >> 16;
			mesh->faces[j].vz = *data;

			if( ++j < mesh->face_count ) {
				mesh->faces[j].vx = (*data++) >> 16;
				mesh->faces[j].vy = *data;
				mesh->faces[j].vz = (*data++) >> 16;
			}
		}

		if( mesh->face_count % 2 ) {
			++data;
		}

		/* Load normals */
		for( j = 0; j < mesh->normal_count; ++j ) {
			mesh->normals[j].vx = *data;
			mesh->normals[j].vy = (*data++) >> 16;
			mesh->normals[j].vz = *data;

			if( ++j < mesh->normal_count ) {
				mesh->normals[j].vx = (*data++) >> 16;
				mesh->normals[j].vy = *data;
				mesh->normals[j].vz = (*data++) >> 16;
			}
		}

		if( mesh->normal_count % 2 ) {
			++data;
		}

		/* Load normal indices */
		for( j = 0; j < mesh->face_count; ++j ) {
			mesh->nidxs[j].vx = *data;
			mesh->nidxs[j].vy = (*data++) >> 16;
			mesh->nidxs[j].vz = *data;

			if( ++j < mesh->face_count ) {
				mesh->nidxs[j].vx = (*data++) >> 16;
				mesh->nidxs[j].vy = *data;
				mesh->nidxs[j].vz = (*data++) >> 16;
			}
		}

		if( mesh->face_count % 2 ) {
			++data;
		}

		/* Load UVs */
		for( j = 0; j < mesh->uv_count; ++j ) {
			u_short u, v;
			u = *data;
			v = (*data++) >> 16;

			mesh->uvs[j].u = (tex_w * u) >> 12;
			mesh->uvs[j].v = (model->tex.prect->h * v) >> 12;
		}

		/* Load texture indices */
		for( j = 0; j < mesh->face_count; ++j ) {
			mesh->uvidxs[j].vx = *data;
			mesh->uvidxs[j].vy = (*data++) >> 16;
			mesh->uvidxs[j].vz = *data;

			if( ++j < mesh->face_count ) {
				mesh->uvidxs[j].vx = (*data++) >> 16;
				mesh->uvidxs[j].vy = *data;
				mesh->uvidxs[j].vz = (*data++) >> 16;
			}
		}

		if( mesh->face_count % 2 ) {
			++data;
		}

		for( j = 0; j < mesh->face_count; ++j ) {
			/* Load face types and colors */
			face.type = *data;

			/* Is Gouraud-shaded? */
			if( face.type & 0x2 ) {
				face.c0.r = (*data) >> 8;
				face.c0.g = (*data) >> 16;
				face.c0.b = (*data++) >> 24;

				face.c1.r = *data;
				face.c1.g = (*data) >> 8;
				face.c1.b = (*data) >> 16;

				face.c2.r = (*data++) >> 24;
				face.c2.g = *data;
				face.c2.b = (*data++) >> 8;
			} else {
				face.color.r = (*data) >> 8;
				face.color.g = (*data) >> 16;
				face.color.b = (*data++) >> 24;

				LOG("%d %d %d\n", face.color.r, face.color.g, face.color.b);
			}

			mesh->face_data[j] = face;
		}
	}

	return data - data_start;
}

void gfx_setup_texture(Model *model) {
	RECT *prect = model->tex.prect;
	RECT *crect = model->tex.crect;

	model->tpage = getTPage(model->tex.mode & 0x3, 0, prect->x, prect->y);
	model->clut = getClut(crect->x, crect->y);
}

void gfx_load_model_texture(Model *model, const char *TEX, u_int *width) {
	if( !TEX ) {
		return;
	}

	image_load(TEX, &model->tex);
	image_upload(&model->tex);

	if( width ) {
		*width = model->tex.prect->w;
		switch( model->tex.mode & 0x3 ) {
		case 0:
			*width *= 2;
			/* fallthrough */
		case 1:
			*width *= 2;
		}
	}

	gfx_setup_texture(model);
}

void gfx_draw_model(Model *model) {
	MATRIX omtx, lmtx;
	u_int i, j;
	Mesh *mesh;

	PushMatrix();

	RotMatrix_gte(&model->rot, &omtx);
	TransMatrix(&omtx, &model->trans);
	ScaleMatrix(&omtx, &model->scale);

	MulMatrix0(&light_matrix, &omtx, &lmtx);
	gte_SetLightMatrix(&lmtx);

	CompMatrixLV(&camera.mat, &omtx, &omtx);

	gte_SetRotMatrix(&omtx);
	gte_SetTransMatrix(&omtx);

	for( i = 0; i < model->mesh_count; ++i ) {
		mesh = &model->meshes[i];

		for( j = 0; j < mesh->face_count; ++j ) {
			Face face;
			face = mesh->face_data[j];

			switch( face.type ) {
			case FACE_POLY_F3:
				p_poly_f3 = (POLY_F3 *)next_primitive;
				if( _prepare_mesh(mesh, j) ) {
					gfx_set_poly_f3(mesh, j);
					next_primitive = (char *)p_poly_f3;
				}
				break;
			case FACE_POLY_FT3:
				p_poly_ft3 = (POLY_FT3 *)next_primitive;
				if( _prepare_mesh(mesh, j) ) {
					gfx_set_poly_ft3(mesh, j, model->tpage, model->clut);
					next_primitive = (char *)p_poly_ft3;
				}
				break;
			case FACE_POLY_G3:
				p_poly_g3 = (POLY_G3 *)next_primitive;
				if( _prepare_mesh(mesh, j) ) { }
				break;
			case FACE_POLY_GT3:
				p_poly_gt3 = (POLY_GT3 *)next_primitive;
				if( _prepare_mesh(mesh, j) ) { }
				break;
			}
		}
	}

	PopMatrix();
}

static inline int _prepare_mesh(Mesh *mesh, u_int i) {
	gte_ldv3(&mesh->verts[mesh->faces[i].vx], &mesh->verts[mesh->faces[i].vy],
		&mesh->verts[mesh->faces[i].vz]);

	gte_rtpt();

	gte_nclip();
	gte_stopz(&gte_result);
	if( gte_result > 0 ) {
		return 0;
	}

	gte_avsz3();
	gte_stotz(&otz);

	otz >>= 2;
	if( otz <= 5 || otz >= OT_LENGTH ) {
		return 0;
	}

	return 1;
}

void gfx_set_poly_f3(Mesh *mesh, const u_int i) {
	const CVECTOR c = mesh->face_data[i].color;

	setPolyF3(p_poly_f3);

	gte_stsxy3(&p_poly_f3->x0, &p_poly_f3->x1, &p_poly_f3->x2);
	if( _test_tri_clip((DVECTOR *)&p_poly_f3->x0, (DVECTOR *)&p_poly_f3->x1,
			(DVECTOR *)&p_poly_f3->x2) ) {
		return;
	}

	setRGB0(p_poly_f3, c.r, c.g, c.b);

	gte_stdp(&gte_result);
	gte_stflg(&gte_result);

	gte_stszotz(&gte_result);
	gte_result /= 3;

	if( gte_result > 0 && gte_result < OT_LENGTH ) {
		addPrim(ot[active_buffer] + otz, p_poly_f3);
	}

	++p_poly_f3;
}

void gfx_set_poly_ft3(Mesh *mesh, const u_int i, u_short tpage, u_short clut) {
	const CVECTOR c = mesh->face_data[i].color;

	setPolyFT3(p_poly_ft3);

	gte_stsxy3(&p_poly_ft3->x0, &p_poly_ft3->x1, &p_poly_ft3->x2);
	if( _test_tri_clip((DVECTOR *)&p_poly_ft3->x0, (DVECTOR *)&p_poly_ft3->x1,
			(DVECTOR *)&p_poly_ft3->x2) ) {
		return;
	}

	setRGB0(p_poly_ft3, c.r, c.g, c.b);

	gte_ldrgb(&p_poly_ft3->r0);
	gte_ldv0(&mesh->normals[mesh->nidxs[i].vx]);
	gte_ncs();

	gte_strgb(&p_poly_ft3->r0);

	setUV3(p_poly_ft3, mesh->uvs[mesh->uvidxs[i].vx].u,
		mesh->uvs[mesh->uvidxs[i].vx].v, mesh->uvs[mesh->uvidxs[i].vy].u,
		mesh->uvs[mesh->uvidxs[i].vy].v, mesh->uvs[mesh->uvidxs[i].vz].u,
		mesh->uvs[mesh->uvidxs[i].vz].v);

	p_poly_ft3->tpage = tpage;
	p_poly_ft3->clut = clut;

	gte_stszotz(&gte_result);
	gte_result /= 3;

	if( gte_result > 0 && gte_result < OT_LENGTH ) {
		addPrim(ot[active_buffer] + otz, p_poly_ft3);
	}

	++p_poly_ft3;
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

static int _test_clip(short x, short y) {
	int result = 0;

	if( x < screen.x ) {
		result |= CLIP_LEFT;
	}

	if( x >= (screen.x + (screen.w - 1)) ) {
		result |= CLIP_RIGHT;
	}

	if( y < screen.y ) {
		result |= CLIP_UP;
	}

	if( y >= (screen.y + (screen.h - 1)) ) {
		result |= CLIP_DOWN;
	}

	return result;
}

static int _test_tri_clip(DVECTOR *v0, DVECTOR *v1, DVECTOR *v2) {
	int c0, c1, c2;

	c0 = _test_clip(v0->vx, v0->vy);
	c1 = _test_clip(v1->vx, v1->vy);

	if( (c0 & c1) == 0 ) {
		return 0;
	}

	c2 = _test_clip(v2->vx, v2->vy);

	if( (c1 & c2) == 0 ) {
		return 0;
	}

	if( (c0 & c2) == 0 ) {
		return 0;
	}

	return 1;
}
