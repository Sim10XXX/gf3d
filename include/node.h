#ifndef __NODE_H__
#define __NODE_H__

#include "simple_logger.h"
#include "entity.h"

#define MAX_CHECKPOINTS 32
#define COLLECTION_RADIUS 13
#define COLLECTION_RADIUS_SQUARED COLLECTION_RADIUS*COLLECTION_RADIUS


typedef struct
{
	GFC_Vector3D	position;
	Uint8			active; //AI should chase active nodes
	Uint8			checkpointNode; //If the node references a checkpoint or finish
	Entity*			block; //The referenced checkpoint or finish
}Node;
// The efficacy of the nodes entirely depends on how they are placed
// The AI will go through the nodes mostly inorder of their creation

void node_system_close();

void node_system_init(Uint32 maxNodes);

void set_nodes(Node* nodeList);

void reset_node_states();

void draw_all_nodes();

GFC_Vector3D get_next_node(GFC_Vector3D playerpos); //gives position of the node that should be chased, also handles deactivating nodes

void collect_checkpoint(); //Called when AI touches a checkpoint. This will mess things up if the AI somehow collects
							//checkpoints out of the intended order

#endif