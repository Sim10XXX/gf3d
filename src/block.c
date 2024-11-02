#include "simple_logger.h"

//#include "gfc_matrix.h"

#include "entity.h"

#include "player.h"

#include "block.h"

#include "node.h"

typedef struct
{
	Uint8 touched;	//for the player
	Uint8 AItouched; //for the AI
}blockData;

void block_reset(Entity* self) {
	if (!self) {
		slog("invalid self");
		return;
	}
	blockData* bdata = self->data;
	if (!bdata) {
		slog("no data");
		return;
	}
	bdata->touched = 0;
	bdata->AItouched = 0;
}

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
	if (!player) {
		slog("invalid player");
		return;
	}
	playerData* pdata = player->data;
	if (!pdata) {
		slog("no player data");
		return;
	}

	if (bdata->touched && pdata->playerType == playertype_player) {
		return;
	}
	if (bdata->AItouched && pdata->playerType == playertype_ai) {
		return;
	}

	
	mapData* mdata = pdata->mapData;
	if (!mdata) {
		slog("no map data");
		return;
	}
	if (pdata->playerType == playertype_player) {
		bdata->touched = 1;
		mdata->currentCheckpoints++;
	}
	else if (pdata->playerType == playertype_ai) {
		bdata->AItouched = 1;
		mdata->currentCheckpoints++;
		collect_checkpoint();
		//slog("collected by ai");
	}
	mdata->lastCheckpoint = self;
}

void finish_touch(Entity* self, Entity* player) {
	if (!self) {
		slog("invalid self");
		return;
	}
	blockData* bdata = self->data;
	if (!bdata) {
		slog("no data");
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

	if (bdata->touched && pdata->playerType == playertype_player) {
		return;
	}
	if (bdata->AItouched && pdata->playerType == playertype_ai) {
		return;
	}

	
	mapData* mdata = pdata->mapData;
	if (!mdata) {
		slog("no map data");
		return;
	}
	//slog("Curr: %i, total: %i", mdata->currentCheckpoints, mdata->totalCheckpoints);
	if (mdata->currentCheckpoints != mdata->totalCheckpoints) return;
	pdata->gameState = 1;
	if (pdata->playerType == playertype_ai) {
		collect_checkpoint();
	}
}

void grass_touch(Entity* self, Entity* player) {
	if (!self) {
		slog("invalid self");
		return;
	}
	blockData* bdata = self->data;
	if (!bdata) {
		slog("no data");
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

	
	pdata->friction += 0.05;
}

void booster_touch(Entity* self, Entity* player) {
	if (!self) {
		slog("invalid self");
		return;
	}
	blockData* bdata = self->data;
	if (!bdata) {
		slog("no data");
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
	if (bdata->touched && pdata->playerType == playertype_player) {
		return;
	}
	if (bdata->AItouched && pdata->playerType == playertype_ai) {
		return;
	}
	if (pdata->playerType == playertype_player) {
		bdata->touched = 1;
	}
	else if (pdata->playerType == playertype_ai) {
		bdata->AItouched = 1;
	}
	pdata->friction -= 0.05;
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
		block->model = gf3d_model_load_full("models/platform/gate.obj", "models/platform/platform.png");
		break;
	case 5:
		block->model = gf3d_model_load_full("models/platform/checkpoint.obj", "models/platform/blue.png");
		block->colliding = 2;
		block->touch = checkpoint_touch;
		break;
	case 6:
		block->model = gf3d_model_load_full("models/platform/finish.obj", "models/platform/red.png");
		block->colliding = 2;
		block->touch = finish_touch;
		break;
	case 7:
		block->model = gf3d_model_load("models/grass.model");
		block->touch = grass_touch;
		break;
	case 8:
		block->model = gf3d_model_load_full("models/platform/pole.obj", "models/platform/platform.png");
		break;
	case 9:
		block->model = gf3d_model_load("models/booster.model");
		block->touch = booster_touch;
		block->update = block_reset;
		break;
	}
	
		//gf3d_model_load("models/primitives/cube.obj");
	block->free = block_free;
	block->reset = block_reset;
	
}

/*void set_node_id(Entity* self, Uint16 nodeId) {
	if (!self) {
		slog("invalid self");
		return;
	}
	blockData* bdata = self->data;
	if (!bdata) {
		slog("no data");
		return;
	}
	bdata->nodeId = nodeId;
}*/