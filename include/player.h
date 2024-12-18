#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "simple_logger.h"
#include "gfc_audio.h"
#include "entity.h"
#include "map.h"

#define radius_of_player 10
#define key_up 1
#define key_right 2
#define key_down 4
#define key_left 8
#define key_respawn 16
#define key_cycle_up 32
#define key_cycle_down 64
#define playertype_player 0
#define playertype_replay 1
#define playertype_ai 2

#define SLOWMOFACTOR 0.6

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
	Uint8			surface;
	float			camstep;
	Uint16			effectReactorTime;
	Sint8			reactorDir;
	Uint8			effectCruiseControl;
	float			CruiseControlSpeed;
	Uint8			effectEngineOff;
	Uint16			effectSlowMoTime;
	
	Uint16			RPM;
	Uint8			gear;

	GFC_Sound		*silence;
	GFC_Sound		*engineSound;
	GFC_Sound		*_e1;
	GFC_Sound		*_e2;
	GFC_Sound		*_e3;
	GFC_Sound		*_e4;
	GFC_Sound		*_e5;
	GFC_Sound		*_e6;
	GFC_Sound		*_e7;
	GFC_Sound		*_e8;
	GFC_Sound		*_e9;
	GFC_Sound		*_e10;

	Uint8			soundTickRate;
	Uint8			pitchTickRate;
	Uint8			currentlyAccelerating;
	Uint8			currentlyAcceleratingLastFrame;
	Uint8			lastEnginePitch;

	GFC_Vector3D	modelOffset;

	GFC_List		*animationList;
	int				animationFrame;

	Uint8			currentSurface;

	//car stats and stuff
	Uint8			currentCar;

	float			accelMult;
	float			handlingMult;
	float			maxGears;
	float			tractionMult;

}playerData;

Entity *spawn_player(mapData* mdata, Uint8 playerType);
void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity* self);

#endif