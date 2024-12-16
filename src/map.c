#include "simple_logger.h"

#include "entity.h"

#include "block.h"

#include "map.h"

#include "node.h"

#include "gfc_config.h"

#include "gamestate.h"


mapData* load_map_from_cfg(const char* filename) {
	SJson* SJmap, * a, *blocklist, *nodelist;
	Uint8 start;
	int nodeId;
	GFC_Vector3D offset = { 0 };

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

	blocklist = sj_object_get_value(SJmap, "blocks");
	nodelist = sj_object_get_value(SJmap, "nodes");
	if (!blocklist) {
		slog("Invalid map file");
		return 0;
	}

	if (sj_object_get_vector3d(SJmap, "gridoffset", &offset)) {
		gfc_vector3d_scale(offset, offset, 20);
	}
	


	int c = sj_array_get_count(blocklist);
	int i;
	Entity* block;

	Entity* checkpointList[MAX_CHECKPOINTS];
	memset(checkpointList, 0, sizeof(Entity*) * MAX_CHECKPOINTS);
	mdata->hasNodes = 0;
	//GFC_Vector3D grid;
	for (i = 0; i < c; i++) {
		slog("a1");
		a = sj_array_get_nth(blocklist, i);
		if (!a) continue;
		int id;
		if (!sj_object_get_value_as_int(a, "id", &id)) {
			continue;
		}

		block = spawn_block(id);

		if (!block) {
			continue;
		}

		if (sj_object_get_vector3d(a, "grid", &block->position)) {
			gfc_vector3d_scale(block->position, block->position, 20);
			block->position.z += 10;
		}
		else if (!sj_object_get_vector3d(a, "position", &block->position)) {
			block->position = gfc_vector3d(0, 0, 10);
		}

		if (!sj_object_get_vector3d(a, "rotation", &block->rotation)) {
			if (!sj_object_get_vector3d(a, "rotationradians", &block->rotation)) {
				block->rotation = gfc_vector3d(0, 0, 0);
			}
		}
		else {
			gfc_vector3d_scale(block->rotation, block->rotation, GFC_DEGTORAD);
		}
		if (!sj_object_get_vector3d(a, "scale", &block->scale)) {
			block->scale = gfc_vector3d(1, 1, 1);
		}
		gfc_vector3d_add(block->position, block->position, offset);
		//sj_object_get_value_as_int(a, "colliding", &block->colliding);

		//slog("c: % i", c);
		//slog("x: %f, y: %f, z: %f", gfc_vector3d_to_slog(block->position));
		//GFC_HALF_PI;
		//slog("id: %i, number: %i", id, i);
		if (id == 5) {
			mdata->totalCheckpoints++;
		}
		slog("a2");
		if (sj_object_get_value_as_uint8(a, "start", &start)) {
			if (start) {
				mdata->startBlock = block;
				mdata->lastCheckpoint = block;
				if (get_editormode()) {
					block->colormod = GFC_COLOR_RED;
				}
				bdata_set_start(block, 1);
			}
		}

		if (sj_object_get_value_as_int(a, "nodeId", &nodeId)) {
			if (nodeId >= MAX_CHECKPOINTS) {
				slog("nodeId too large");
				continue;
			}
			checkpointList[nodeId] = block;
			slog("Block Nodeid: %i", nodeId);
		}

	}
	if (!mdata->startBlock && !get_editormode()) {
		slog("Invalid map, no start found");
		return NULL;
	}

	c = sj_array_get_count(nodelist);
	if (get_editormode()) {
		c = UINT8_MAX;
	}
	//Entity* block;
	//GFC_Vector3D grid;

	node_system_init(c);
	Node* nodeList;
	nodeList = gfc_allocate_array(sizeof(Node), c);
	if (!nodeList) {
		slog("Failed to alloc nodeList during map parsing");
		//return mdata;
	}
	if (c > 0) {
		mdata->hasNodes = 1;
	}
	for (i = 0; i < c; i++) {
		a = sj_array_get_nth(nodelist, i);
		if (!a) continue;

		if (sj_object_get_vector3d(a, "grid", &nodeList[i].position)) {
			gfc_vector3d_scale(nodeList[i].position, nodeList[i].position, 20);
			nodeList[i].position.z += 10;
		}
		else if (!sj_object_get_vector3d(a, "position", &nodeList[i].position)) {
			nodeList[i].position = gfc_vector3d(0, 0, 10);
		}
		//slog("x: %f, y: %f, z: %f", gfc_vector3d_to_slog(nodeList[i].position));
		if (sj_object_get_value_as_int(a, "nodeId", &nodeId)) {
			if (nodeId >= MAX_CHECKPOINTS) {
				slog("nodeId too large");
				continue;
			}
			nodeList[i].block = checkpointList[nodeId];
			nodeList[i].checkpointNode = 1;
			slog("Node Nodeid: %i", nodeId);
		}
		gfc_vector3d_add(nodeList[i].position, nodeList[i].position, offset);
		slog("x: %f, y: %f, z: %f", gfc_vector3d_to_slog(nodeList[i].position));
		nodeList[i]._inuse = 1;
	}
	set_nodes(nodeList);

	if (nodeList) {
		free(nodeList);
	}
	free(SJmap);
	Entity* stadium = entity_new();
	stadium->model = gf3d_model_load("models/stadium.model");
	stadium->colliding = 1;

	return mdata;
}

int convert_current_entities_into_map(int id) {
	Entity* block;
	SJson* sj, *sjtest, *blockar, *nodear;
	int i;
	char buffer[20];
	delete_duplicate_blocks();

	sj = sj_object_new();
	sj_object_insert(sj, "title", sj_new_str("sample title"));


	blockar = sj_array_new();
	while (block = get_next_block()) {
		sj_array_append(blockar, block_to_json(block));
	}
	sj_object_insert(sj, "blocks", blockar);
	
	nodear = sj_array_new();
	//if (!nodear) {
	//	slog("bruhhh");
	//}
	//slog("sj: %i", nodear->sjtype);
	//slog("fff");
	Node* node;
	int nodeid = 0;
	while (node = iter_get_next_node(&nodeid)) {
		slog("wwd");
		sj_array_append(nodear, node_to_json(node, nodeid));
	}
	//slog("why does it crash");
	sj_object_insert(sj, "nodes", nodear);



	if (id != 0) {
		i = id;
		sprintf(buffer, "maps/map%i.tmap", i);
	}
	else {
		for (i = 0;; i++) {
			sprintf(buffer, "maps/map%i.tmap", i);
			if ((sjtest = sj_load(buffer)) != NULL) {
				sj_free(sjtest);
			}
			else {
				break;
			}
		}
	}
	sj_object_insert(sj, "mapID", sj_new_int(i));
	
	sj_save(sj, buffer);
	sj_free(sj);
	slog("a4");


	return i;
}

mapData* load_empty_map(int mapID) {
	mapData* mdata;
	mdata = gfc_allocate_array(sizeof(mapData), 1);
	if (!mdata) {
		slog("Could not allocate mapData");
		return 0;
	}
	mdata->mapID = mapID;

	Entity* stadium = entity_new();
	stadium->model = gf3d_model_load("models/stadium.model");
	stadium->colliding = 1;

	return mdata;
}