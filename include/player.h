#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "simple_logger.h"
#include "entity.h"

#define radius_of_player 10

typedef struct
{
	Uint8			cameraMode;
	GFC_Vector3D	positionVelocity;
	GFC_Vector3D	rotationVelocity;
	//wheel positions xyz
	GFC_Vector3D	wheelFL; //front left
	GFC_Vector3D	wheelFR; //front right
	GFC_Vector3D	wheelRL; //rear left
	GFC_Vector3D	wheelRR; //rear right
	GFC_Vector3D	relativePos[4]; //positions of the wheels relative to the car (should be constant)
	int				framecount;
}playerData;

Entity *spawn_player();
void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity* self);

#endif