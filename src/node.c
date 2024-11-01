#include "node.h"
#include "gf3d_draw.h"

typedef struct
{
	Node* nodeList; //big a** list
	Uint32 nodeMax;
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
		node_manager.nodeList[i].position = nodeList[i].position;
		node_manager.nodeList[i].active = 1;
		node_manager.nodeList[i].checkpointNode = nodeList[i].checkpointNode;
		node_manager.nodeList[i].block = nodeList[i].block;
	}
}

void reset_node_states() {
	int i;
	for (i = 0; i < node_manager.nodeMax; i++)
	{
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
	}
	return node_manager.nodeList[i].position;
}

void collect_checkpoint() {
	int i;
	for (i = 0; i < node_manager.nodeMax; i++)
	{
		if (!node_manager.nodeList[i].active) continue;
		node_manager.nodeList[i].active = 0;
		if (node_manager.nodeList[i].checkpointNode) {
			break;
		}
	}
}