#ifndef GUARD_PSGAME_CAMERA_H_
#define GUARD_PSGAME_CAMERA_H_

#include <sys/types.h>

#include <libgte.h>
#include <libgpu.h>

typedef struct _Camera {
	SVECTOR rot;

	VECTOR trans;
	VECTOR ttrans;

	MATRIX mat;
} Camera;

void cam_init(Camera *camera);
void cam_update(Camera *camera);

#endif // !GUARD_PSGAME_CAMERA_H_
