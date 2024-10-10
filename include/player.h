#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "simple_logger.h"
#include "entity.h"

Entity *spawn_player();
void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity* self);

#endif