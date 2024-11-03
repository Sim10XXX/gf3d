#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "simple_logger.h"
#include "entity.h"
#include "map.h"

#define radius_of_player 10
#define key_up 1
#define key_right 2
#define key_down 4
#define key_left 8
#define key_respawn 16
#define playertype_player 0
#define playertype_replay 1
#define playertype_ai 2
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

	GFC_Vector3D	aboveFL; //front left
	GFC_Vector3D	aboveFR; //front right
	GFC_Vector3D	aboveRL; //rear left
	GFC_Vector3D	aboveRR; //rear right

	GFC_Vector3D	forward;

	GFC_Vector3D	relativePos[4]; //positions of the wheels relative to the car (should be constant)

	int				framecount;
	int				framedelta;		//comparison to ghost

	Uint8			gameState;

	mapData*		mapData;

	Uint8			playerType;		//0 = player, 1 = replay ghost, 2 = AI ghost

	FILE*			currReplay;

	float			friction;

	GFC_Vector3D	currentNormal; //of the surface the player is in contact with

	Uint8			sliding; //There should be different turning physics if the car is in a sliding state or not

	float			camstep;
}playerData;

Entity *spawn_player(mapData* mdata, Uint8 playerType);
void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity* self);

#endif