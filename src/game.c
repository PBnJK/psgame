/* psgame
 * Game handler
 */

#include "camera.h"

#include "game.h"

Camera camera;

void game_init(void) {
	cam_init(&camera);
}

void game_update(void) {
	cam_update(&camera);
}
