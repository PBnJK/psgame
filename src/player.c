/* psgame
 * Player
 */

#include <sys/types.h>

#include <libgte.h>
#include <libgpu.h>

#include "input.h"
#include "game.h"
#include "trig.h"

#include "player.h"

#define ROT_SPEED (12)

void player_init(Player *player) {
	setVector(&player->rot, 0, 0, 0);

	player->speed = 0;
	player->speed_frac = 0;
}

void player_update(Player *player) {
	player->trot.vx = player->rot.vx >> 12;
	player->trot.vy = player->rot.vy >> 12;
	player->trot.vz = player->rot.vz >> 12;

	/* Look down */
	if( !PAD_P1.up ) {
		player->rot.vx += ONE * ROT_SPEED;
	}

	/* Look up */
	if( !PAD_P1.down ) {
		player->rot.vx -= ONE * ROT_SPEED;
	}

	/* Look left */
	if( !PAD_P1.left ) {
		player->rot.vy += ONE * ROT_SPEED;
	}

	/* Look right */
	if( !PAD_P1.right ) {
		player->rot.vy -= ONE * ROT_SPEED;
	}

	/* Move forwards */
	if( !PAD_P1.triangle ) {
		if( player->speed < 4 ) {
			++player->speed_frac;
			if( player->speed_frac > 50 ) {
				++player->speed;
				player->speed_frac = 0;
			}
		}
	}

	/* Move backwards */
	if( !PAD_P1.cross ) {
		player->trans.vx
			+= ((trig_isin(player->trot.vy) * trig_icos(player->trot.vx)) >> 12)
			<< 2;
		player->trans.vy -= trig_isin(player->trot.vx) << 2;
		player->trans.vz
			-= ((trig_icos(player->trot.vy) * trig_icos(player->trot.vx)) >> 12)
			<< 2;
	}

	/* Strafe left */
	if( !PAD_P1.square ) {
		player->trans.vx -= trig_icos(player->trot.vy) << 2;
		player->trans.vz -= trig_isin(player->trot.vy) << 2;
	}

	/* Strafe Right */
	if( !PAD_P1.circle ) {
		player->trans.vx += trig_icos(player->trot.vy) << 2;
		player->trans.vz += trig_isin(player->trot.vy) << 2;
	}

	/* Slide up */
	if( !PAD_P1.l1 ) {
		player->trans.vx
			-= ((trig_isin(player->trot.vy) * trig_isin(player->trot.vx)) >> 12)
			<< 2;
		player->trans.vy -= trig_icos(player->trot.vx) << 2;
		player->trans.vz
			+= ((trig_icos(player->trot.vy) * trig_isin(player->trot.vx)) >> 12)
			<< 2;
	}

	/* Slide down */
	if( !PAD_P1.r1 ) {
		player->trans.vx
			+= ((trig_isin(player->trot.vy) * trig_isin(player->trot.vx)) >> 12)
			<< 2;
		player->trans.vy += trig_icos(player->trot.vx) << 2;
		player->trans.vz
			-= ((trig_icos(player->trot.vy) * trig_isin(player->trot.vx)) >> 12)
			<< 2;
	}

	camera.trans.vx = -player->trans.vx >> 12;
	camera.trans.vy = -player->trans.vy >> 12;
	camera.trans.vz = -player->trans.vz >> 12;

	camera.rot = player->trot;

	player_move(player);
}

void player_display(Player *player) { }

void player_move(Player *player) {
	player->trans.vx
		-= ((trig_isin(player->trot.vy) * trig_icos(player->trot.vx)) >> 12)
		<< player->speed;
	player->trans.vy += trig_isin(player->trot.vx) << 2;
	player->trans.vz
		+= ((trig_icos(player->trot.vy) * trig_icos(player->trot.vx)) >> 12)
		<< player->speed;
}
