#include "node.h"
#include "gf3d_draw.h"
#include "block.h"
#include "gfc_config.h"

typedef struct
{
	Node* nodeList; //big a** list
	Uint32 nodeMax;
	Uint32 currIndex; //for get next node
}NodeManager;

static NodeManager node_manager = { 0 };

void node_system_close()
{
	int i;
	free(node_manager.nodeList);
	memset(&node_manager, 0, sizeof(NodeManager));
}

void node_system_init(Uint32 maxNodes) {
	if (node_manager.nodeList)
	{
		slog("node manager already exists");
		return;
	}
	if (!maxNodes)
	{
		slog("cannot allocate 0 nodes for the node manager");
		return;
	}
	node_manager.nodeList = gfc_allocate_array(sizeof(Node), maxNodes);
	if (!node_manager.nodeList)
	{
		slog("failed to allocate %i nodes for the node manager", maxNodes);
		return;
	}
	node_manager.nodeMax = maxNodes;
	atexit(node_system_close);
}

void set_nodes(Node* nodeList) {
	int i;
	for (i = 0; i < node_manager.nodeMax; i++)
	{
		if (nodeList[i]._inuse) {
			node_manager.nodeList[i].position = nodeList[i].position;
			node_manager.nodeList[i].active = 1;
			node_manager.nodeList[i].checkpointNode = nodeList[i].checkpointNode;
			node_manager.nodeList[i].block = nodeList[i].block;
			node_manager.nodeList[i]._inuse = 1;
		}
	}
}

void reset_node_states() {
	int i;
	for (i = 0; i < node_manager.nodeMax; i++)
	{
		if (!node_manager.nodeList[i]._inuse) continue;
		node_manager.nodeList[i].active = 1;
	}
}

void draw_all_nodes() {
	int i;
	GFC_Sphere sphere = { 0 };
	GFC_Color color;
	sphere.r = 1;
	for (i = 0; i < node_manager.nodeMax; i++)
	{
		if (!node_manager.nodeList[i]._inuse) continue;
		if (!node_manager.nodeList[i].active) {
			color = gfc_color(0.5, 0.5, 0.5, 0.5);
		}
		else if (node_manager.nodeList[i].checkpointNode) {
			color = gfc_color(0.5, 1, 1, 0.8);
		}
		else{
			color = gfc_color(1, 1, 0, 0.8);
		}
		gf3d_draw_sphere_solid(sphere, node_manager.nodeList[i].position, gfc_vector3d(0,0,0), gfc_vector3d(1, 1, 1), color, gfc_color(0, 0, 0, 0));
	}
	
}

GFC_Vector3D get_next_node(GFC_Vector3D playerpos) {
	int i;
	for (i = 0; i < node_manager.nodeMax; i++)
	{
		if (!node_manager.nodeList[i]._inuse) continue;
		if (!node_manager.nodeList[i].active) continue;
		
		
		if (node_manager.nodeList[i].checkpointNode) break;
		if (i == node_manager.nodeMax - 1) break;
		if (gfc_vector3d_magnitude_between_squared(node_manager.nodeList[i].position, playerpos) < COLLECTION_RADIUS_SQUARED) {
			node_manager.nodeList[i].active = 0;
			i++;
			break;
		}
		if (gfc_vector3d_magnitude_between_squared(node_manager.nodeList[i + 1].position, playerpos) <
			gfc_vector3d_magnitude_between_squared(node_manager.nodeList[i].position, playerpos)) {
			node_manager.nodeList[i].active = 0;
			i++;
			break;
		}
		//slog("x: %f, y: %f, z: %f", gfc_vector3d_to_slog(node_manager.nodeList[i].position));
		break;
	}
	return node_manager.nodeList[i].position;
}

void collect_checkpoint() {
	int i;
	for (i = 0; i < node_manager.nodeMax; i++)
	{
		if (!node_manager.nodeList[i]._inuse) continue;
		if (!node_manager.nodeList[i].active) continue;
		node_manager.nodeList[i].active = 0;
		if (node_manager.nodeList[i].checkpointNode) {
			break;
		}
	}
}

void node_respawn_from_checkpoint() {
	int i;
	for (i = 0; i < node_manager.nodeMax; i++)
	{
		if (!node_manager.nodeList[i]._inuse) continue;
		if (node_manager.nodeList[i].active) break;
	}
	i--;
	for (; i > 0; i--)
	{
		if (!node_manager.nodeList[i]._inuse) continue;
		if (!node_manager.nodeList[i].checkpointNode) {
			node_manager.nodeList[i].active = 1;
		}
		else {
			break;
		}
	}
}

Node* spawn_node(GFC_Vector3D pos) {
	int i;
	Entity* block;
	for (i = 0; i < node_manager.nodeMax; i++)
	{
		if (node_manager.nodeList[i]._inuse) continue;
		node_manager.nodeList[i]._inuse = 1;
		node_manager.nodeList[i].position = pos;
		node_manager.nodeList[i].active = 1;
		block = get_closest_block(1, 5, pos);
		if (block) {
			node_manager.nodeList[i].checkpointNode = 1;
			node_manager.nodeList[i].block = block;
			bdata_set_node_id(block, i);
		}
		return &node_manager.nodeList[i];
	}
	slog("reached max nodes");
	return 0;
}

void node_free(Node* node) {
	if (!node) return;
	memset(node, 0, sizeof(Node));
}

void delete_near_nodes(GFC_Vector3D pos, float range) {
	int i;
	for (i = 0; i < node_manager.nodeMax; i++)
	{
		if (!node_manager.nodeList[i]._inuse) continue;
		if (gfc_vector3d_magnitude_between_squared(pos, node_manager.nodeList[i].position) < range * range) {
			node_free(&node_manager.nodeList[i]);
		}
	}
}

Node* iter_get_next_node(int* nodeid) {
	int i;
	slog("c");
	for (i = node_manager.currIndex; i < node_manager.nodeMax; i++, node_manager.currIndex++) {
		if (!node_manager.nodeList[i]._inuse) continue;
		slog("c1");
		*nodeid = i;
		node_manager.currIndex++;
		return &node_manager.nodeList[i];
	}
	node_manager.currIndex = 0;
	return NULL;
}

SJson* node_to_json(Node* node, int nodeid) {
	SJson* save;
	if (!node) return NULL;
	save = sj_object_new();
	if (!save) return NULL;

	GFC_Vector3D tvec;
	gfc_vector3d_copy(tvec, node->position);
	tvec.z -= 10;
	tvec.x /= 20;
	tvec.y /= 20;
	tvec.z /= 20;
	sj_object_insert(save, "grid", sj_vector3d_new(tvec));

	if (node->checkpointNode) {
		//Entity* block = find_block_with_nodeid(nodeid);
		sj_object_insert(save, "nodeId", sj_new_int(nodeid));
	}
	return save;
}