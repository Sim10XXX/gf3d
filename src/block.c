#include "simple_logger.h"

//#include "gfc_matrix.h"

#include "entity.h"

#include "player.h"

#include "block.h"

typedef struct
{
	Uint8 touched;
}blockData;

void checkpoint_touch(Entity* self, Entity* player) {
	if (!self) {
		slog("invalid self");
		return;
	}
	blockData* bdata = self->data;
	if (!bdata) {
		slog("no data");
		return;
	}
	if (bdata->touched) {
		return;
	}

	if (!player) {
		slog("invalid player");
		return;
	}
	playerData* pdata = player->data;
	if (!pdata) {
		slog("no player data");
		return;
	}
	bdata->touched = 1;
	mapData* mdata = pdata->mapData;
	if (!mdata) {
		slog("no map data");
		return;
	}
	mdata->currentCheckpoints++;
	mdata->lastCheckpoint = self;
}

void block_free(Entity* self) {
	if (!self) {
		slog("invalid self pointer for block_free");
		return;
	}
	if (!self->data) {
		slog("no block data to free");
		return;
	}
	free(self->data);
}

Entity* spawn_block(int id) {
	Entity* block = entity_new();
	blockData* bdata;
	if (!block) {
		slog("Could not create block entity");
		return 0;
	}
	bdata = gfc_allocate_array(sizeof(blockData), 1);
	if (!bdata) {
		slog("Could not allocate block data");
		return 0;
	}
	//memset(bdata, 0, sizeof(blockData));

	block->position.z = -2;

	//block->hitbox = gfc_box(block->position.x - 2, block->position.y - 2, block->position.z - 0.1, 4, 4, 0.2);
	//GFC_Vector3D p1, p2, p3;
	//gfc_vector3d_add(p1, block->position,gfc_vector3d(-2.0,-2.0,0.1));
	//gfc_vector3d_add(p2, block->position, gfc_vector3d(2.0, -2.0, 0.1));
	//gfc_vector3d_add(p3, block->position, gfc_vector3d(-2.0, 2.0, 0.1));

	//bdata->hitbox = gfc_triangle(p1,p2,p3);
	block->data = bdata;
	block->scale = gfc_vector3d(1, 1, 1);
	block->colliding = 1;
	switch (id) {
	case 1:
		block->model = gf3d_model_load("models/platform.model");
		break;
	case 2:
		block->model = gf3d_model_load("models/ramp.model");
		break;
	case 3:
		block->model = gf3d_model_load("models/quarterpipe.model");
		break;
	case 4:
		block->model = gf3d_model_load_full("models/platform/checkpoint.obj", "models/platform/blue.png");
		block->colliding = 2;
		block->touch = checkpoint_touch;
		break;
	case 5:
		block->model = gf3d_model_load_full("models/platform/gate.obj", "models/platform/platform.png");
	}
	
		//gf3d_model_load("models/primitives/cube.obj");
	block->free = block_free;
	
}

