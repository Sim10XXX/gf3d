#ifndef __BLOCK_H__
#define __BLOCK_H__

#include "simple_logger.h"
#include "entity.h"

Entity* spawn_block(int id);

//void player_think(Entity* self);
//void player_update(Entity* self);

void block_free(Entity* self);


#endif