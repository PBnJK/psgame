#ifndef GUARD_PSGAME_ACTOR_H_
#define GUARD_PSGAME_ACTOR_H_

#include <sys/types.h>

#include "gfx.h"

typedef enum _ActorType {
	ACTOR_NONE = 0,
	ACTOR_PLAYER = 1,
	ACTOR_FRIENDLY_SHIP = 2,
	ACTOR_ENEMY_SHIP = 3,
} ActorType;

typedef struct _Actor {
	ActorType type;
	union {
		Mesh mesh;
	};
} Actor;

#endif // !GUARD_PSGAME_ACTOR_H_
