#ifndef GUARD_PSGAME_GFX_H_
#define GUARD_PSGAME_GFX_H_

#include <sys/types.h>

#include <libgte.h>
#include <libgpu.h>

#include "camera.h"
#include "sprite.h"
#include "text.h"

#define MAX_MESHES (32)

#define MAX_FACES (256)
#define MAX_VERTS (MAX_FACES * 2)

#define MAX_UVIDXS (64)
#define MAX_UVS (MAX_UVIDXS * 2)

#define MAX_NIDXS (64)
#define MAX_NORMALS (MAX_NIDXS * 2)

typedef struct _UV {
	u_char u, v;
} UV;

typedef struct _Mesh {
	/* Vertex array */
	u_short vertex_count;
	SVECTOR verts[MAX_VERTS];

	/* Face array */
	u_short face_count;
	SVECTOR faces[MAX_FACES];

	/* UV array */
	u_short uv_count;
	SVECTOR uvidxs[MAX_UVIDXS];
	UV uvs[MAX_UVS];

	/* Normal array */
	u_short normal_count;
	SVECTOR nidxs[MAX_NIDXS];
	SVECTOR normals[MAX_NORMALS];
} Mesh;

typedef struct _Model {
	SVECTOR rot;
	VECTOR trans;
	VECTOR scale;

	/* Texture */
	TIM_IMAGE tex;
	u_short tpage;
	u_short clut;

	u_short mesh_count;
	Mesh meshes[MAX_MESHES];
} Model;

/* Initializes the GFX subsystem */
void gfx_init(void);

/* Quits the GFX subsystem */
void gfx_quit(void);

/* Blits the ordering tables to the screen */
void gfx_display(void);

/* Initializes a model */
void gfx_init_model(Model *model);

/* Loads a model from a file */
u_int gfx_load_model(Model *model, const char *FILENAME, const char *TEX);

/* Loads a model from a data stream */
u_int gfx_load_model_from_ptr(Model *model, u_long *data, const char *TEX);

/* Sets up a model texture */
void gfx_setup_texture(Model *model);

/* Loads a texture into a model */
void gfx_load_model_texture(Model *model, const char *TEX, u_int *width);

/* Draws a texture */
void gfx_draw_model(Camera *camera, Model *model);

/* Sets a POLY_F3 for rendering */
void gfx_set_poly_f3(void);

/* Sets a POLY_FT3 for rendering */
void gfx_set_poly_ft3(Mesh *mesh, const u_int i, u_short tpage, u_short clut);

/* Sets a Sprite for rendering */
void gfx_set_sprite(Sprite *spr);

/* Sets a Font for rendering */
void gfx_set_font(Font *font, u_short x, u_short y);

/* Sets a TPAGE for rendering */
void gfx_set_tpage(u_short tpage);

/* Sets an STP for rendering */
void gfx_set_stp(u_short stp);

#endif // !GUARD_PSGAME_GFX_H_
