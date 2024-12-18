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
	Uint8			currentSurface;
	Entity			*selectedBlock;
	mapData			*mdata;
	Uint8			nodeMode;
	Node			*selectedNode;
	Uint16			cycleMin;
	Uint16			cycleMax;
	Uint8			movingBlockMode;
	Uint8			placementStep; //for placing moving blocks
	GFC_Vector3D	*movingBlockPos;

}editorData;

Entity* editor_spawn(mapData *mdata);

void editor_free(Entity *self);

#endif