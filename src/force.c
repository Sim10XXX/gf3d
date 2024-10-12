#include "simple_logger.h"
#include "gfc_matrix.h"
#include "force.h"

#define radius_of_player_squared 100
#define torque_mult 0.1 //used to scale the rotational force by some constant

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

float calculate_torque(Force2D f) {
	if (f.origin.x == 0.0 && f.origin.y == 0.0) return 0.0;
	if (f.forceVector.x == 0.0 && f.forceVector.y == 0.0) return 0.0;

	float L; //distance from 0,0
	L = gfc_vector2d_magnitude(f.origin);

	//set forceVector to be relative to 0,0 instead of the force's origin
	//gfc_vector2d_sub(f.forceVector, f.forceVector, f.origin);

	//angle of the line L // angle to rotate forceVector by
	float theta = 90 - acos(f.origin.x / L);

	//rotate forceVector
	GFC_Vector2D temp = f.forceVector;
	f.forceVector.x = temp.x * cos(theta) + temp.y * sin(theta);

	//don't need the y, since we are only using the x for the torque
	//f.forceVector.y = temp.x * sin(theta) + temp.y * cos(theta);
	return L * f.forceVector.x * torque_mult;
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

	//If a force is not directed at the center of gravity, it will have less of an effect on positionVelocity, 
	//,because the force will also apply rotationalVelocity in the torque calculation
	//thus the force is scaled by  inverseDistSquared  (for now)

	gfc_vector3d_scale(delta, force.forceVector, inverseDistSquared);
	gfc_vector3d_add(pdata->positionVelocity, pdata->positionVelocity, delta);
	


	//calculate torque (change in rotational velocity)
	GFC_Vector3D torque = {0};

	//rotation about the z axis
	Force2D f;
	f.origin = gfc_vector2d(force.origin.x, force.origin.y);
	f.forceVector = gfc_vector2d(force.forceVector.x, force.forceVector.y);
	torque.z = calculate_torque(f);

	//rotation about the y axis (or x i guess)
	f.origin = gfc_vector2d(force.origin.x, force.origin.z);
	f.forceVector = gfc_vector2d(force.forceVector.x, force.forceVector.z);
	torque.x = calculate_torque(f);

	//rotation about the x axis (or y i guess)
	f.origin = gfc_vector2d(force.origin.y, force.origin.z);
	f.forceVector = gfc_vector2d(force.forceVector.y, force.forceVector.z);
	torque.y = calculate_torque(f);

	gfc_vector3d_add(pdata->rotationVelocity, pdata->rotationVelocity, torque);
}

