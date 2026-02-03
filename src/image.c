/* psgame
 * TIM images
 */

#include <sys/types.h>

#include <libgte.h>
#include <libgpu.h>

#include "common.h"
#include "cd.h"
#include "mem.h"

#include "image.h"

void image_load(const char *FILENAME, TIM_IMAGE *tim) {
	u_long *data = cd_load_file_with_name(FILENAME);

	OpenTIM(data);
	ReadTIM(tim);

	mem_free(data);
}

void image_upload(TIM_IMAGE *tim) {
	LoadImage(tim->prect, tim->paddr);
	DrawSync(0);

	/* Does this TIM image have a CLUT? */
	if( tim->mode & 0x8 ) {
		LoadImage(tim->crect, tim->caddr);
		DrawSync(0);
	}
}
