/* psgame
 * Camera
 */

#include <sys/types.h>

#include <libgte.h>
#include <libgpu.h>

#include "camera.h"

void cam_init(Camera *camera) {
	setVector(&camera->trans, 0, 0, 0);
	setVector(&camera->ttrans, 0, 0, 0);
	setVector(&camera->rot, 0, 0, 0);
}

void cam_update(Camera *camera) {
	RotMatrix(&camera->rot, &camera->mat);

	ApplyMatrixLV(&camera->mat, &camera->trans, &camera->ttrans);
	TransMatrix(&camera->mat, &camera->ttrans);
}
