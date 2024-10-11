#include "simple_logger.h"
#include "gfc_matrix.h"
#include "force.h"

#define radius_of_player_squared 100

void normalize_force(Force* force) {
	if (!force) return;
	Force absForce;
	GFC_Vector3D forceDelta;
	float x, y, z, mult;
	if (force->origin.x == 0 || force->origin.y == 0 || force->origin.z == 0) return;

	//Get absolute value of force
	absForce.origin = gfc_vector3d(fabs(force->origin.x), fabs(force->origin.y), fabs(force->origin.z));
	absForce.forceVector = gfc_vector3d(fabs(force->forceVector.x), fabs(force->forceVector.y), fabs(force->forceVector.z));

	//divide origin by forceVector
	if (absForce.forceVector.x != 0) x = absForce.origin.x / absForce.forceVector.x;
	else x = 9999999999;
	if (absForce.forceVector.y != 0) y = absForce.origin.y / absForce.forceVector.y;
	else y = 9999999999;
	if (absForce.forceVector.z != 0) z = absForce.origin.z / absForce.forceVector.z;
	else z = 9999999999;

	//determine whether x, y, or z is the closest to zero and set the mult
	if (x < y && x < z) {
		if (force->origin.x < 0 ^ force->forceVector.x < 0) {
			mult = -x;
		}
		else {
			mult = x;
		}
	}
	else if(y < z) {
		if (force->origin.y < 0 ^ force->forceVector.y < 0) {
			mult = -y;
		}
		else {
			mult = y;
		}
	}
	else {
		if (force->origin.z < 0 ^ force->forceVector.z < 0) {
			mult = -z;
		}
		else {
			mult = z;
		}
	}

	// move the force along its vector
	gfc_vector3d_scale(forceDelta, force->forceVector, mult);
	gfc_vector3d_add(force->origin, force->origin, forceDelta);
}

void apply_force(Force force, playerData* pdata) {
	if (!pdata) return;
	normalize_force(&force);
	float distSquared = (force.origin.x * force.origin.x) + (force.origin.y * force.origin.y) + (force.origin.z * force.origin.z);
	GFC_Vector3D delta;

	if (distSquared > radius_of_player_squared) {
		slog("force somehow outside of radius");
		return;
	}
	float inverseDistSquared = radius_of_player_squared - distSquared;

	distSquared /= radius_of_player_squared;
	inverseDistSquared /= radius_of_player_squared;

	gfc_vector3d_scale(delta, force.forceVector, inverseDistSquared);
	gfc_vector3d_add(pdata->positionVelocity, pdata->positionVelocity, delta);
	
}