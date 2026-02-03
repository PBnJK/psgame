#ifndef GUARD_PSGAME_IMAGE_H_
#define GUARD_PSGAME_IMAGE_H_

#include <sys/types.h>

#include <libgte.h>
#include <libgpu.h>

void image_load(const char *FILENAME, TIM_IMAGE *tim);
void image_upload(TIM_IMAGE *tim);

#endif // !GUARD_PSGAME_IMAGE_H_
