#ifndef GUARD_PSGAME_PLAYER_H_
#define GUARD_PSGAME_PLAYER_H_

#include <sys/types.h>

#include <libgte.h>

typedef struct _Player {
	SVECTOR trot;
	VECTOR rot;

	VECTOR trans;
	VECTOR scale;

	int speed;
	int speed_frac;
} Player;

/* Initializes the player */
void player_init(Player *player);

/* Processes the player (input, movement, etc.) */
void player_update(Player *player);

/* Draws the player to the screen (UI & ship) */
void player_display(Player *player);

/* Applies forward velocity */
void player_move(Player *player);

#endif // !GUARD_PSGAME_PLAYER_H_
