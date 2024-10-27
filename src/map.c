#include "simple_logger.h"

#include "entity.h"

#include "block.h"

#include "map.h"


mapData* load_map_from_cfg(const char* filename) {
	SJson* SJmap, * a;
	Uint8 start;
	SJmap = sj_load(filename);
	if (!SJmap) {
		slog("Couldn't load filename");
		return 0;
	}
	mapData* mdata;
	mdata = gfc_allocate_array(sizeof(mapData), 1);
	if (!mdata) {
		slog("Could not allocate mapData");
		return 0;
	}

	sj_object_get_value_as_int(SJmap, "mapID", &mdata->mapID);

	SJmap = sj_object_get_value(SJmap, "blocks");
	if (!SJmap) {
		slog("Invalid map file");
		return 0;
	}

	


	int c = sj_array_get_count(SJmap);
	int i;
	Entity* block;
	for (i = 0; i < c; i++) {
		a = sj_array_get_nth(SJmap, i);
		if (!a) continue;
		int id;
		sj_object_get_value_as_int(a, "id", &id);

		block = spawn_block(id);

		if (!block) {
			continue;
		}

		sj_object_get_vector3d(a, "position", &block->position);
		sj_object_get_vector3d(a, "rotation", &block->rotation);
		sj_object_get_vector3d(a, "scale", &block->scale);
		//sj_object_get_value_as_int(a, "colliding", &block->colliding);

		//slog("c: % i", c);
		//slog("x: %f, y: %f, z: %f", gfc_vector3d_to_slog(block->position));
		//GFC_HALF_PI;
		//slog("id: %i, number: %i", id, i);
		if (id == 5) {
			mdata->totalCheckpoints++;
		}
		if (sj_object_get_value_as_uint8(a, "start", &start)) {
			if (start) {
				mdata->startBlock = block;
				mdata->lastCheckpoint = block;
			}
		}
	}
	if (!mdata->startBlock) {
		slog("Invalid map, no start found");
		return NULL;
	}
	return mdata;
}