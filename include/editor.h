#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "simple_logger.h"
#include "entity.h"
#include "map.h"
#include "node.h"

typedef struct
{
	float			cameraPitch; //angles to determine where the camera is placed
	float			cameraYaw;
	float			cameraDistance;			//the distance at that angle
	Uint8			cameraMode;
	float			camstep;
	int				currcycle;
	Entity			*selectedBlock;
	mapData			*mdata;
	Uint8			nodeMode;
	Node			*selectedNode;

}editorData;

Entity* editor_spawn(mapData *mdata);

void editor_free(Entity *self);

#endif