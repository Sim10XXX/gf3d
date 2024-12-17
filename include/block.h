#ifndef __BLOCK_H__
#define __BLOCK_H__

#include "simple_logger.h"
#include "entity.h"

#define CHECKPOINT_ID 5
#define FINISH_ID 6
#define EFFECT_GATE_REACTOR_ID 10
#define EFFECT_GATE_SLOWMO_ID 11
#define EFFECT_GATE_CRUISECONTROL_ID 12
#define EFFECT_GATE_ENGINEOFF_ID 13
#define EFFECT_GATE_RESET_ID 14

#define SURFACE_ROAD 0
#define SURFACE_P_GRASS 1
#define SURFACE_DIRT 2
#define SURFACE_ICE 4
#define SURFACE_WOOD 8

//#define MAX_ID 14

Entity* spawn_block(int id);

//void player_think(Entity* self);
//void player_update(Entity* self);

void block_free(Entity* self);

//void set_node_id(Entity* self, Uint16 nodeId);

SJson* block_to_json(Entity* self);


void bdata_set_start(Entity* self, Uint8 v);

Uint8 bdata_get_start(Entity* self);

Uint8 bdata_get_id(Entity* self);

void bdata_set_node_id(Entity* self, Uint8 v);

Uint8 bdata_get_node_id(Entity* self);

Uint8 compare_blocks(Entity* a, Entity* b);

#endif