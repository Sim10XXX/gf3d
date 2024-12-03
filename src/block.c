#include "simple_logger.h"

//#include "gfc_matrix.h"

#include "entity.h"

#include "gfc_config.h"

#include "player.h"

#include "block.h"

#include "node.h"

//#include "gamestate.h"

typedef struct
{
	Uint8 touched;	//for the player
	Uint8 AItouched; //for the AI
	Uint8 replaytouched; //for the replay ghost
	Uint8 id;
	Uint8 start;
	Uint8 nodeid;
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
	bdata->replaytouched = 0;
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
	if (bdata->touched && pdata->playerType == playertype_player) {
		return;
	}
	if (bdata->AItouched && pdata->playerType == playertype_ai) {
		return;
	}
	if (bdata->replaytouched && pdata->playerType == playertype_replay) {
		return;
	}
	if (pdata->playerType == playertype_player) {
		bdata->touched = 1;
	}
	else if (pdata->playerType == playertype_ai) {
		bdata->AItouched = 1;
	}
	else if (pdata->playerType == playertype_replay) {
		bdata->replaytouched = 1;
	}
	pdata->surface = 1;
	//slog("grasstouch");
	pdata->friction += 0.6;
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
	if (bdata->replaytouched && pdata->playerType == playertype_replay) {
		return;
	}
	if (pdata->playerType == playertype_player) {
		bdata->touched = 1;
	}
	else if (pdata->playerType == playertype_ai) {
		bdata->AItouched = 1;
	}
	else if (pdata->playerType == playertype_replay) {
		bdata->replaytouched = 1;
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
	block->collisionRadius = 20;
	block->isBlock = 1;
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
	case CHECKPOINT_ID:
		block->model = gf3d_model_load_full("models/platform/checkpoint.obj", "models/platform/blue.png");
		block->colliding = 2;
		block->touch = checkpoint_touch;
		break;
	case FINISH_ID:
		block->model = gf3d_model_load_full("models/platform/finish.obj", "models/platform/red.png");
		block->colliding = 2;
		block->touch = finish_touch;
		break;
	case 7:
		block->model = gf3d_model_load("models/grass.model");
		block->touch = grass_touch;
		block->update = block_reset;
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
	bdata->id = id;
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

SJson* block_to_json(Entity* self) {
	SJson* save, *templist;
	if (!self) return NULL;
	blockData *bdata = self->data;
	if (!bdata) return NULL;
	save = sj_object_new();
	if (!save) return NULL;
	sj_object_insert(save, "id", sj_new_int(bdata->id));
	if (bdata->nodeid) {
		sj_object_insert(save, "nodeId", sj_new_int(bdata->nodeid));
	}
	if (bdata->start) {
		sj_object_insert(save, "start", sj_new_int(bdata->start));
	}
	GFC_Vector3D tvec;
	gfc_vector3d_copy(tvec, self->position);
	tvec.z -= 10;
	tvec.x /= 20;
	tvec.y /= 20;
	tvec.z /= 20;
	sj_object_insert(save, "grid", sj_vector3d_new(tvec));

	if (gfc_vector3d_magnitude(self->rotation) != 0.0) {
		gfc_vector3d_copy(tvec, self->rotation);
		gfc_vector3d_scale(tvec, tvec, GFC_RADTODEG);
		sj_object_insert(save, "rotation", sj_vector3d_new(tvec));
	}
	
	//save->get_string
	//slog("%s", save->get_string(save)->text);
	return save;
}

void bdata_set_start(Entity* self, Uint8 v) {
	if (!self) return;
	blockData *bdata = self->data;
	if (!bdata) return;
	bdata->start = v;
}

Uint8 bdata_get_start(Entity* self) {
	if (!self) return 0;
	blockData* bdata = self->data;
	if (!bdata) return 0;
	return bdata->start;
}

Uint8 compare_blocks(Entity* a, Entity* b) {
	blockData* adata, * bdata;
	if (!a) return 0;
	if (!b) return 0;
	adata = a->data;
	bdata = b->data;
	if (!adata) return 0;
	if (!bdata) return 0;

	if (adata->id != bdata->id) return 0;
	if (!gfc_vector3d_compare(a->position, b->position)) return 0;
	if (!gfc_vector3d_compare(a->rotation, b->rotation)) return 0;

	return 1;
}

Uint8 bdata_get_id(Entity* self) {
	if (!self) return 0;
	blockData* bdata = self->data;
	if (!bdata) return 0;
	return bdata->id;
}

void bdata_set_node_id(Entity* self, Uint8 v) {
	if (!self) return;
	blockData* bdata = self->data;
	if (!bdata) return;
	bdata->nodeid = v;
}

Uint8 bdata_get_node_id(Entity* self) {
	if (!self) return 0;
	blockData* bdata = self->data;
	if (!bdata) return 0;
	return bdata->nodeid;
}