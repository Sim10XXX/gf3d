#ifndef __FORCE_H__
#define __FORCE_H__

#include "simple_logger.h"
#include "gfc_matrix.h"
#include "player.h"

#define radius_of_player_squared radius_of_player*radius_of_player

// The only physics object is the player, 
// all forces that exist are ones that are affecting the player, 
// any force that doesn't hit the center of gravity will apply rotational velocity

typedef struct
{
	GFC_Vector3D	origin;			//The position of the force relative to the player
	GFC_Vector3D	forceVector;	//The direction and magnitude as a vector, (0,0,0) means no force
}Force3D;

typedef struct
{
	GFC_Vector2D	origin;			
	GFC_Vector2D	forceVector;
}Force2D;			//Used for torque calculations

/*
* "move" a force along it's forceVector so that some part of the origin, either the x, y, or z is 0
* I think this will be useful for calculating rotational velocity produced by a force
*/

Force3D force3d(GFC_Vector3D origin, GFC_Vector3D forceVector);

void normalize_force(Force3D* force);

void apply_force(Force3D force, playerData *pdata);

#endif