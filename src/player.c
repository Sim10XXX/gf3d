#include "simple_logger.h"

#include "gfc_matrix.h"
#include "gf3d_camera.h"
#include "gfc_input.h"
#include "entity.h"
#include "player.h"
#include "force.h"
#include "replay.h"
#include "node.h"
#include "gamestate.h"

#define FRICTION 0.005
#define wheel_radius 1
#define normal_force_mult 0.59
#define replay_on 0
#define ai_on 1


void project(Entity* self, playerData* pdata, Entity* projectionEnt, playerData* projectionData);

int ailogic(Entity* self, GFC_Vector3D nodepos) {
	playerData* pdata;
	if (!self) {
		return 0;
	}
	pdata = self->data;

	if (!pdata) {
		slog("no pdata");
		return 0;
	}
	int inputs = 0;

	GFC_Vector2D nodeDir, velocityDir, facingDir, nodeDirClockwise90;
	float velocitymag = gfc_vector3d_magnitude(pdata->positionVelocity);

	gfc_vector2d_sub(nodeDir, nodepos, self->position);
	gfc_vector2d_normalize(&nodeDir);

	velocityDir = gfc_vector3dxy(pdata->positionVelocity);
	gfc_vector2d_normalize(&velocityDir);

	facingDir = gfc_vector2d_from_angle(self->rotation.z);
	gfc_vector2d_normalize(&facingDir);

	nodeDirClockwise90 = gfc_vector2d(-nodeDir.y, nodeDir.x);

	//should I press forward?
	if (velocitymag < 0.2) {
		inputs |= key_up;
	}
	if (gfc_vector2d_dot_product(facingDir, nodeDir) > 0.5) {
		inputs |= key_up;
	}
	//should I brake?
	if (gfc_vector2d_dot_product(velocityDir, nodeDir) < 0) {
		inputs |= key_down;
		//slog("brake");
	}

	//always try to face the next node

	float rotationdeadzone = 0.1 * gfc_random()+0.08;

	
	if (gfc_vector2d_dot_product(facingDir, nodeDir) < 0) { //if facing away, remove the deadzone
		rotationdeadzone = 0;
	}

	if (gfc_vector2d_dot_product(facingDir, nodeDirClockwise90) < rotationdeadzone) {
		inputs |= key_left;
	}
	else if (gfc_vector2d_dot_product(facingDir, nodeDirClockwise90) > rotationdeadzone) {
		inputs |= key_right;
	}

	//should I respawn?
	if (velocitymag < 0.3) {
		pdata->framecount++;
	}
	else {
		pdata->framecount = 0;
	}
	if (pdata->framecount > 150) {
		inputs |= key_respawn;
		pdata->framecount = 0;
	}

	return inputs;
}


int player_get_inputs(Entity* self) {
	playerData* pdata;
	if (!self) {
		return 0;
	}
	pdata = self->data;

	if (!pdata) {
		slog("no pdata");
		return 0;
	}
	int inputs = 0;
	if (pdata->playerType == playertype_replay) {
		inputs = read_replay(pdata->currReplay);
		//slog("inputs: %i", inputs);
		return inputs;
	}
	if (pdata->playerType == playertype_ai) {
		GFC_Vector3D nodepos;
		nodepos = get_next_node(self->position);
		//slog("x: %f, y: %f, z: %f", gfc_vector3d_to_slog(nodepos));
		inputs = ailogic(self, nodepos);
		return inputs;
	}


	if (gfc_input_command_down("respawn")) {
		inputs |= key_respawn;
	}
	if (gfc_input_command_down("walkforward")) {
		inputs |= key_up;
	}
	if (gfc_input_command_down("walkback")) {
		inputs |= key_down;
	}
	if (gfc_input_command_down("walkright")) {
		inputs |= key_right;
	}
	if (gfc_input_command_down("walkleft")) {
		inputs |= key_left;
	}
	return inputs;
}

void player_free(Entity* self) 
{
	if (!self) {
		slog("invalid self pointer for player_free");
		return;
	}
	playerData* pdata = self->data;
	if (!pdata) {
		slog("no player data to free");
		return;
	}
	mapData* mdata = pdata->mapData;
	if (!mdata) {
		slog("no map data to free");
	}
	else {
		free(mdata);
	}

	free(pdata);
	memset(self, 0, sizeof(Entity));
}

void player_reset(Entity* self) {
	playerData* pdata;
	if (!self) {
		return;
	}
	pdata = self->data;
	
	if (!pdata) {
		slog("no pdata");
		return;
	}
	mapData* mdata = pdata->mapData;
	if (!mdata)return;
	//memset(pdata, 0, sizeof(playerData));
	pdata->positionVelocity = gfc_vector3d(0, 0, 0);
	pdata->rotationVelocity = gfc_vector3d(0, 0, 0);

	pdata->currentNormal = gfc_vector3d(0, 0, 0);

	pdata->sliding = 0; 
	pdata->surface = 0;


	self->position = mdata->startBlock->position;
	self->rotation = mdata->startBlock->rotation;
	
	
}
/*GFC_Vector4D ToQuaternion(GFC_Vector3D angles);
GFC_Vector3D ToEulerAngles(GFC_Vector4D q);
GFC_Vector4D multiplyQuaternions(GFC_Vector4D q1, GFC_Vector4D q2);*/
void player_turn(Entity* self, float turnmult) {
	if (!self)return;
	playerData* pdata = self->data;
	if (!pdata)return;
	
	if (pdata->currentNormal.x == 0 && pdata->currentNormal.y == 0 && pdata->currentNormal.z == 0) return;
	
	float mag = gfc_vector3d_magnitude(pdata->positionVelocity);
	GFC_Vector3D velocityDelta, negativePVelocity, scaledNormal, previouspVelocity;
	

	scaledNormal = pdata->currentNormal;
	gfc_vector3d_set_magnitude(&scaledNormal, 0.04 * turnmult);
	//slog("sliding: %i", pdata->sliding);
	if (pdata->sliding && mag > 0.1){
		gfc_vector3d_sub(self->rotation, self->rotation, scaledNormal);
		return;
	}
	previouspVelocity = pdata->positionVelocity;
	//goto test;
	negativePVelocity = pdata->positionVelocity;

	gfc_vector3d_cross_product(&velocityDelta, pdata->positionVelocity, scaledNormal);
	if (mag < 1) {
		gfc_vector3d_scale(negativePVelocity, negativePVelocity, -0.033);
		gfc_vector3d_scale(velocityDelta, velocityDelta, mag);
	}
	else{
		gfc_vector3d_set_magnitude(&negativePVelocity, -0.015);
		if (mag > 1.5) {
			gfc_vector3d_scale(velocityDelta, velocityDelta, 1.5/mag);
		}
	}

	gfc_vector3d_add(velocityDelta, velocityDelta, negativePVelocity);

	gfc_vector3d_add(pdata->positionVelocity, pdata->positionVelocity, velocityDelta);

	//gfc_angle
	float theta;
	theta = acos(gfc_vector3d_dot_product(pdata->positionVelocity, previouspVelocity) / 
		(gfc_vector3d_magnitude(pdata->positionVelocity) * gfc_vector3d_magnitude(previouspVelocity)));
	 //!pdata->sliding
	if (isnan(theta)) {
		//slog("oop");
		return;
	}
	gfc_vector3d_set_magnitude(&scaledNormal, theta);
	//slog("theta: %f", theta);
	//slog("pos: x: %f, y: %f, z: %f", gfc_vector3d_to_slog(self->position));
	gfc_vector3d_sub(self->rotation, self->rotation, scaledNormal);
		//self->rotation.z = -gfc_vector2d_angle(gfc_vector3dxy(pdata->positionVelocity));
	
	//float dz, dy, dx;
	//GFC_Vector3D n;
	
	/*dz = turnmult * cos(self->rotation.x) * cos(self->rotation.y);
	dy = turnmult * sin(self->rotation.x);
	dx = turnmult * sin(self->rotation.y);


	dz *= -0.05;
	dy *= 0.05;
	dx *= 0.05;

	self->rotation.z += dz;
	self->rotation.y += dy;
	self->rotation.x += dx;
	n = pdata->currentNormal;
	
	dz = atan2(n.y, n.x);
	dy = acos(n.z);


	self->rotation.z += dz;
	self->rotation.y += dy;
	self->rotation.x += dx;*/
	/*
	slog("og rot: x: %f, y: %f, z: %f", gfc_vector3d_to_slog(self->rotation));
	GFC_Vector3D e, rotateby;
	GFC_Vector4D q, qrotateby;
	e = self->rotation;
	q = ToQuaternion(e);

	rotateby = gfc_vector3d(0, 0, 0.05 * turnmult);
	qrotateby = ToQuaternion(rotateby);
	
	q = multiplyQuaternions(q, qrotateby);

	e = ToEulerAngles(q);
	slog("new rot: x: %f, y: %f, z: %f", gfc_vector3d_to_slog(e));
	self->rotation = e;*/

	//Just add the axis forehead its not that hard
	//gfc_vector3d_set_magnitude(&scaledNormal, 0.05);
	//
	// gfc_vector3d_scale(scaledNormal, scaledNormal, turnmult);
	// 
	//scaledNormal;

	//scaledNormal.z = scaledNormal.z * -1;
	//slog("turnmult: %f", turnmult);
	//slog("normal: x: %f, y: %f, z: %f", gfc_vector3d_to_slog(scaledNormal));
	
}

/*GFC_Vector4D ToQuaternion(GFC_Vector3D angles)
{
	float cy = cos(angles.z * 0.5);
	float sy = sin(angles.z * 0.5);
	float cp = cos(angles.y * 0.5);
	float sp = sin(angles.y * 0.5);
	float cr = cos(angles.x * 0.5);
	float sr = sin(angles.x * 0.5);

	GFC_Vector4D q = { 0 };
	q.w = cr * cp * cy + sr * sp * sy;
	q.x = sr * cp * cy - cr * sp * sy;
	q.y = cr * sp * cy + sr * cp * sy;
	q.z = cr * cp * sy - sr * sp * cy;

	return q;
}

GFC_Vector3D ToEulerAngles(GFC_Vector4D q)
{
	GFC_Vector3D angles = { 0 };

	// roll (x-axis rotation)
	float sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	float cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	angles.x = atan2(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	float sinp = 2 * (q.w * q.y - q.z * q.x);
	if (abs(sinp) >= 1)
	{
		angles.y = copysign(GFC_HALF_PI, sinp);
	}
	else
	{
		angles.y = asin(sinp);
	}

	// yaw (z-axis rotation)
	float siny_cosp = 2 * (q.w * q.z + q.x * q.y);
	float cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
	angles.z = atan2(siny_cosp, cosy_cosp);

	return angles;
}

GFC_Vector4D multiplyQuaternions(GFC_Vector4D q1, GFC_Vector4D q2) {
	GFC_Vector4D result;

	result.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
	result.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
	result.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
	result.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;

	return result;
}*/
void player_think(Entity* self) 
{
	if (is_paused()) return;
	if (!self)return;
	playerData* pdata = self->data;
	if (!pdata)return;
	mapData* mdata = pdata->mapData;
	if (!mdata)return;

	GFC_Vector3D dv = {0};

	pdata->friction = FRICTION;

	if (!pdata->gameState && pdata->playerType != playertype_ai) {
		pdata->framecount++;
	}
	else if (pdata->currReplay) { //Reached finish line, update saved replay with temp if faster
		FILE* mapreplay;
		mapreplay = open_replay(mdata->mapID);
		if (mapreplay) {
			int mapreplayframes = get_replay_size(mapreplay)/4 + 1;
			slog("replay time: %.2f, my time: %.2f", mapreplayframes / 30.0, pdata->framecount / 30.0);
			if (pdata->framecount < mapreplayframes) {
				save_temp_replay(mdata->mapID, pdata->currReplay);
			}
			pdata->currReplay = NULL;
			fclose(mapreplay);
			pdata->framedelta = pdata->framecount - mapreplayframes;
		}
		else {
			save_temp_replay(mdata->mapID, pdata->currReplay);
			pdata->currReplay = NULL;
			pdata->framedelta = 0;
		}
		
	}

	
	if (pdata->framecount == 1 && pdata->playerType != playertype_ai) { //skip first frame because otherwise bad things happen
		return;
	}
	if (gfc_input_command_pressed("freecam")) {
		if (pdata->cameraMode == 7) {
			pdata->cameraMode = 0;
		}
		else {
			pdata->cameraMode = 7;
		}
	}
	if (gfc_input_command_down("restart")) {
		//Entity* startBlock = mdata->startBlock;
		//player_reset(self);
		entity_reset();
		//self->position = startBlock->position;
		//self->rotation = startBlock->rotation;

		mdata->currentCheckpoints = 0;
		mdata->lastCheckpoint = mdata->startBlock;
		pdata->gameState = 0;
		pdata->framecount = 1;

		if (pdata->playerType == playertype_replay) {
			if (pdata->currReplay) {
				fclose(pdata->currReplay);
				pdata->currReplay = open_replay(mdata->mapID);
			}
			
		}
		else if (pdata->playerType == playertype_player){
			pdata->currReplay = refresh_temp_replay(pdata->currReplay);
		}
		else if (pdata->playerType == playertype_ai) {
			reset_node_states();
		}
		

		return;
	}
	int inputs = 0;
	if (!pdata->gameState) {
		inputs = player_get_inputs(self);
		if (pdata->playerType == playertype_player) {
			append_to_temp_replay(inputs, pdata->currReplay);
		}
	}
	if (pdata->playerType == playertype_player) {
		if (pdata->cameraMode && inputs) {
			pdata->camstep += 0.1;
		}
		else {
			pdata->camstep = 0.2;
		}
		if (pdata->cameraMode) {
			gf3d_camera_set_move_step(pdata->camstep);
		}
	}
	
	
	if ((key_down & inputs) && !pdata->gameState) {
		pdata->sliding = 1;
		pdata->friction += 0.2;
		//slog("yep");
	}
	if (pdata->surface) {
		pdata->sliding = 1;
		//slog("grass");
	}
	if ((key_respawn & inputs) && !pdata->gameState) { //could remove the checks for gamestate
		Entity* checkpointBlock = mdata->lastCheckpoint;
		player_reset(self);
		self->position = checkpointBlock->position;
		self->rotation = checkpointBlock->rotation;
		if (pdata->playerType == playertype_ai) {
			node_respawn_from_checkpoint();
		}
		return;
	}

	if ((key_up & inputs) && !pdata->gameState) {

		//gfc_vector2d_scale(dv, gfc_vector2d_from_angle(self->rotation.z),0.05);
		
		//apply_force(
		//	force3d(gfc_vector3d(0, 0, 0),
		//		gfc_vector3d(dv.x, dv.y, 0)), //sin(self->rotation.x)*0.05)
		//	self, 0);
		GFC_Vector3D normalRight;
		gfc_vector3d_sub(normalRight, pdata->wheelRR, pdata->wheelRL);
		gfc_vector3d_cross_product(&dv, pdata->currentNormal, normalRight);
		float acc;
		acc = 0.06 / sqrtf(gfc_vector3d_magnitude(pdata->positionVelocity)+4);
		gfc_vector3d_set_magnitude(&dv, acc);

		apply_force(
			force3d(gfc_vector3d(0, 0, 0),
				dv), //sin(self->rotation.x)*0.05)
			self, 0);
		
	}
	if ((key_right & inputs) && !pdata->gameState) {
		//pdata->rotationVelocity.z = -0.05;
		//apply_force_as_turn(
		//	force3d(gfc_vector3d(0, radius_of_player, 0),
		//		gfc_vector3d(-0.05, 0, 0)),
		//	self);
		player_turn(self, 1);
	}
	
	if ((key_left & inputs) && !pdata->gameState) {
		//pdata->rotationVelocity.z = 0.05;
		//apply_force_as_turn(
		//	force3d(gfc_vector3d(0, radius_of_player, 0),
		//		gfc_vector3d(0.05, 0, 0)),
		//	self);
		player_turn(self, -1);
	}
	if (!(key_left & inputs) && !(key_right & inputs)) {
		pdata->rotationVelocity.z = 0;
	}
	//entity_check_collision(self);

	//Add gravity force
	//force.origin = gfc_vector3d(1, 1, 0);
	//force.forceVector = gfc_vector3d(0, 0, -0.001);

	apply_force(
		force3d(gfc_vector3d(0, 0, 0),//pdata->relativePos[0],
			gfc_vector3d(0, 0, -0.02)),
		self, 0);

	//downforce

	GFC_Vector3D down, forward;
	float downmag;
	gfc_vector3d_sub(down, pdata->wheelFL, pdata->aboveFL);
	gfc_vector3d_sub(forward, pdata->wheelFL, pdata->wheelRL);
	downmag = gfc_vector3d_dot_product(forward, pdata->positionVelocity) * 0.005;
	//slog("downmag: %f", downmag);
	if (downmag > 0.05) {
		downmag = 0.05;
	}
	else if (downmag < 0) {
		downmag = 0;
	}
	if (downmag>0){
		gfc_vector3d_set_magnitude(&down, downmag);
		apply_force(
			force3d(gfc_vector3d(0, 0, 0),//pdata->relativePos[0],
				down),
			self, 0);
	}
	

	pdata->surface = 0;
	//pdata->rotationVelocity.x = 0.01;
	GFC_Vector3D* wheel;
	int i, j, k, l;
	int equalcount;
	//GFC_Vector3D vlist[collisions_max];
	GFC_Vector4D vlistlist[4][collisions_max];
	memset(&vlistlist, 0, sizeof(GFC_Vector4D) * collisions_max * 4);

	Force3D forceQueue[4 * collisions_max];
	memset(&forceQueue, 0, sizeof(Force3D) * collisions_max * 4);
	int forceQueueC = 0;


	GFC_Vector3D finalv[4];
	Entity p;
	Entity* projection = &p;
	playerData projectionData;
	memset(&projectionData, 0, sizeof(playerData));
	memset(projection, 0, sizeof(Entity));
	projection->scale = self->scale;
	project(self, pdata, projection, &projectionData);

	GFC_Vector3D* pdataWheel = &pdata->wheelFL;
	//slog("projection x: %f, y: %f, z: %f", projection->position.x, projection->position.y, projection->position.z);

	GFC_Matrix4 playerMatrix, wheelMatrix;
	for (i = 0, wheel = &projectionData.wheelFL; i < 4; i++, wheel++) {
		gfc_matrix4_from_vectors(
			playerMatrix,
			projection->position,
			projection->rotation,
			projection->scale);
		gfc_matrix4_from_vectors(
			wheelMatrix,
			pdata->relativePos[i],
			gfc_vector3d(0, 0, 0),
			gfc_vector3d(1, 1, 1));

		gfc_matrix4_multiply(
			wheelMatrix,
			wheelMatrix,
			playerMatrix);

		gfc_matrix4_to_vectors(
			wheelMatrix,
			wheel,
			NULL,
			NULL);
		GFC_Vector3D a = *wheel;
		GFC_Vector3D b = pdataWheel[i];
		gfc_vector3d_sub(finalv[i], a, b);
	}

	//wheel = &pdata->wheelFL;
	wheel = &projectionData.wheelFL;

	for (i = 0; i < 4; i++, wheel++) {
		check_player_collision(self, gfc_sphere(wheel->x, wheel->y, wheel->z, wheel_radius),&vlistlist[i]);
		//slog("Wheel x: %f, y: %f, z: %f", wheel->x, wheel->y, wheel->z);
	}

	/*GFC_Vector3D* vlistpointer;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < collisions_max; j++) {
			if (gfc_vector4d_compare(vlistlist[i][j], gfc_vector4d(0, 0, 0, 0))) {
				continue;
			}
			vlistpointer = &vlistlist[i][j];
			gfc_vector3d_normalize(vlistpointer);
			slog("normaled");
		}
	}*/

	//get rid of duplicates, which occur when colliding with multiple triangles at the same angle
	for (i = 0; i < 4; i++) {
		for (j = 0; j < collisions_max; j++) {
			if (gfc_vector3d_compare(vlistlist[i][j], gfc_vector3d(0, 0, 0))) {
				continue;
			}
			for (k = j + 1; k < collisions_max; k++) {
				if (gfc_vector3d_compare(vlistlist[i][j], vlistlist[i][k])) {
					gfc_vector3d_set(vlistlist[i][j], 0, 0, 0);
					//slog("dupe removed");
					break;
				}
			}
		}
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j < collisions_max; j++) {
			if (vlistlist[i][j].x == 0 && vlistlist[i][j].y == 0 && vlistlist[i][j].z == 0) continue;
			//gfc_vector3d_set_magnitude(&vlist[j], 0.001);

			//Since the force from vlist is supposed to be the direction of the normal force,
			//We need to calculate the magnitude by using the player's current velocity

			float mag = gfc_vector3d_dot_product(finalv[i], vlistlist[i][j]) / gfc_vector3d_magnitude(gfc_vector4dxyz(vlistlist[i][j]));
			mag *= -normal_force_mult;
			//mag = normal_force_mult;
			equalcount = 1;
			for (k = 0; k < 4; k++) {
				if (k == i) continue;
				for (l = 0; l < collisions_max; l++) {
					if (gfc_vector3d_compare(vlistlist[i][j], vlistlist[k][l])) {
						equalcount++;
						break;
					}
				}
			}
			mag /= equalcount;

			gfc_vector3d_set_magnitude(&vlistlist[i][j], mag);

			/*apply_force(
				force3d(pdata->relativePos[i],
					vlistlist[i][j]),
				self);*/
			GFC_Vector3D velement = gfc_vector4dxyz(vlistlist[i][j]);
			//GFC_Vector3D angles = {0};
			//GFC_Matrix4 rotmatrix;
			/*float theta = self->rotation.x;
			GFC_Matrix3 xrotmatrix =
			{ 1, 0, 0,
			0, cos(theta), -sin(theta),
			0, sin(theta), cos(theta) };

			theta = self->rotation.y;
			GFC_Matrix3 yrotmatrix =
			{ cos(theta), 0, sin(theta),
			0, 1, 0,
			-sin(theta),0 , cos(theta) };

			theta = self->rotation.z;
			GFC_Matrix3 zrotmatrix =
			{ cos(theta), -sin(theta), 0,
			sin(theta), cos(theta), 0,
			0, 0, 1 };
			
			gfc_matrix3_multiply_v(&velement, xrotmatrix, velement);
			gfc_matrix3_multiply_v(&velement, yrotmatrix, velement);
			gfc_matrix3_multiply_v(&velement, zrotmatrix, velement);*/
			//gfc_matrix4_from_vectors(
			//	rotmatrix,
			//	gfc_vector3d(0, 0, 0),
			//	self->rotation,
			//	gfc_vector3d(1, 1, 1));
			//apply_matrix(rotmatrix, &velement);

			//slog("angles: x:%f, y:%f, z:%f",gfc_vector3d_to_slog(angles));
			if (velement.z < 0) {
				//slog("z is negative");
				//velement = gfc_vector3d(-velement.x, -velement.y, -velement.z);
			}
			//GFC_Vector3D forigin, *temp;
			//temp = &pdata->wheelFL;
			//temp += i;

			//gfc_vector3d_sub(forigin, (*temp), self->position);
			forceQueue[forceQueueC] = force3d(pdata->relativePos[i], gfc_vector4dxyz(vlistlist[i][j]));
			forceQueueC++;
			//slog("hit #%i", forceQueueC);
			
		}
	}
	GFC_Vector3D normalSum = {0};
	for (i = 0; i < 4 * collisions_max; i++) {
		apply_force(forceQueue[i], self, 0);
		gfc_vector3d_add(normalSum, normalSum, forceQueue[i].forceVector);
		//slog("forcev x: %f, y: %f, z: %f", gfc_vector3d_to_slog(forceQueue[i].forceVector));
	}

	
	/*gfc_vector3d_sub(finalv, pdata->wheelFL, self->position);
	gfc_vector3d_cross_product(&finalv, pdata->rotationVelocity, finalv);
	gfc_vector3d_add(finalv, finalv, pdata->positionVelocity);
	slog("WheelFL x: %f, y: %f, z: %f",finalv.x, finalv.y, finalv.z);*/
	
	
	//slog("projectiondata x: %f, y: %f, z: %f", projectionData.wheelFL.x, projectionData.wheelFL.y, projectionData.wheelFL.z);
	//slog("pdata x: %f, y: %f, z: %f", pdata->wheelFL.x, pdata->wheelFL.y, pdata->wheelFL.z);
	//slog("WheelFL x: %f, y: %f, z: %f", finalv[0].x, finalv[0].y, finalv[0].z);
	gfc_vector3d_normalize(&normalSum);
	pdata->currentNormal = normalSum; //gfc_vector3d(0, 0, 1);
	GFC_Vector3D gripForce;
	GFC_Vector3D facingDir;
	facingDir = gfc_vector3d(0, 1, 0);
	GFC_Matrix4 pmatrix;
	gfc_matrix4_from_vectors(
		pmatrix,
		gfc_vector3d(0,0,0),
		self->rotation,
		self->scale);
	apply_matrix(pmatrix, &facingDir);

	gfc_vector3d_cross_product(&gripForce, pdata->currentNormal, facingDir);
	float gripforcemag = gfc_vector3d_dot_product(gripForce, pdata->positionVelocity) * -0.1;
	gfc_vector3d_set_magnitude(&gripForce, gripforcemag);
	apply_force(
		force3d(gfc_vector3d(0, 0, 0),//pdata->relativePos[0],
			gripForce),
		self, 0);
	if (gripforcemag < 0.01) {
		pdata->sliding = 0;
	}

	
}


void move_player(Entity* self) 
{
	if (!self)return;
	if (!self->data)return;
	playerData* pdata = self->data;
	gfc_vector3d_add(self->position, pdata->positionVelocity, self->position);
	gfc_vector3d_add(self->rotation, pdata->rotationVelocity, self->rotation);
	
	//self->hitbox = gfc_box(self->position.x - 2, self->position.y - 2, self->position.z - 2, 4, 4, 4);

}

void project(Entity* self, playerData* pdata, Entity* projectionEnt, playerData* projectionData) {
	if (!self || !pdata || !projectionEnt || !projectionData) return;
	projectionEnt->position = self->position;
	projectionEnt->rotation = self->rotation;
	projectionData->rotationVelocity = pdata->rotationVelocity;
	projectionData->positionVelocity = pdata->positionVelocity;
	projectionEnt->data = projectionData;
	move_player(projectionEnt);
}

void velocity_update(Entity* self) {
	GFC_Vector3D deltaPV, deltaRV;
	
	if (!self)return;
	if (!self->data)return;
	playerData* pdata = self->data;

	//friction

	gfc_vector3d_scale(deltaPV, pdata->positionVelocity, pdata->friction);
	gfc_vector3d_scale(deltaRV, pdata->rotationVelocity, FRICTION);

	gfc_vector3d_sub(pdata->positionVelocity, pdata->positionVelocity, deltaPV);
	gfc_vector3d_sub(pdata->rotationVelocity, pdata->rotationVelocity, deltaRV);

	if (gfc_vector3d_magnitude(pdata->positionVelocity) <= 0.001) {
		gfc_vector3d_clear(pdata->positionVelocity);
	}
	if (gfc_vector3d_magnitude(pdata->rotationVelocity) <= 0.001) {
		gfc_vector3d_clear(pdata->rotationVelocity);
	}

	//gravity

	//pdata->positionVelocity.z -= 0.002;
}

/*void update_hitbox(Entity* self) {
	if (!self)return;
	if (!self->data)return;
	playerData* pdata = self->data;
	pdata->hitbox = gfc_box(self->position.x - 2, self->position.y - 2, self->position.z - 2, 4, 4, 4);
}*/

void collision(Entity* self) {
	//gfc_box_overlap();
}

void apply_friction(Entity* self) {
	playerData* pdata;
	if (!self) return;
	if (!self->data)return;
	pdata = self->data;
	GFC_Vector3D fvec;
	gfc_vector3d_scale(fvec, pdata->positionVelocity, pdata->friction);
	if (gfc_vector3d_magnitude(fvec) > 0.01) {
		gfc_vector3d_set_magnitude(&fvec, 0.01);
	}
	gfc_vector3d_sub(pdata->positionVelocity, pdata->positionVelocity, fvec);

	gfc_vector3d_scale(fvec, pdata->rotationVelocity, FRICTION);
	if (gfc_vector3d_magnitude(fvec) > 0.01) {
		gfc_vector3d_set_magnitude(&fvec, 0.01);
	}
	gfc_vector3d_sub(pdata->rotationVelocity, pdata->rotationVelocity, fvec);

}

void player_update(Entity* self)
{
	GFC_Vector3D lookTarget, camera, dir = { 0 };
	playerData* pdata;
	if (is_paused()) return;
	if (!self) return;
	if (!self->data)return;
	pdata = self->data;

	move_player(self);
	//velocity_update(self);
	//update_hitbox(self);
	apply_friction(self);
	
	if (pdata->playerType == playertype_player) {
		set_framecount(pdata->framecount);
		set_speed(_cvt_ftoi_fast(gfc_vector3d_magnitude(pdata->positionVelocity) * 100));
	}

	gfc_vector3d_copy(lookTarget, self->position);
	lookTarget.z += 1;
	dir.y = 30.0;

	if (pdata->playerType == playertype_player && pdata->cameraMode == 0) {
		gfc_vector3d_rotate_about_z(&dir, self->rotation.z);
		gfc_vector3d_sub(camera, self->position, dir);
		camera.z += 10;
		gf3d_camera_look_at(lookTarget, &camera);
	}
	else if (pdata->playerType == playertype_player && pdata->cameraMode == 7) {
	    gf3d_camera_controls_update();
	}
	

	//update wheel pos
	GFC_Matrix4 playerMatrix, wheelMatrix; //inplayerMatrix;

	/*GFC_Vector3D relativePos[4] = {
		gfc_vector3d(-1, 2, 0),
		gfc_vector3d(1, 2, 0),
		gfc_vector3d(-1, -2, 0),
		gfc_vector3d(1, -2, 0) };*/

	//pdata->wheelFL = self->position;

	GFC_Vector3D* wheel;
	int i;
	gfc_matrix4_from_vectors(
		playerMatrix,
		self->position,
		self->rotation,
		self->scale);
	for (i = 0, wheel = &pdata->wheelFL; i < 4; i++, wheel++) {
		gfc_vector3d_copy((*wheel), pdata->relativePos[i]);
		apply_matrix(playerMatrix, wheel);
	}
	for (i = 0, wheel = &pdata->aboveFL; i < 4; i++, wheel++) {
		gfc_vector3d_copy((*wheel), pdata->relativePos[i]);
		wheel->z += 1;
		apply_matrix(playerMatrix, wheel);
	}
	pdata->forward = gfc_vector3d(0, 1, 0);
	apply_matrix(playerMatrix, &pdata->forward);
}

int player_draw(Entity* self)
{
	GFC_Matrix4 matrix;
	playerData* pdata;
	if (!self) return;
	if (!self->data)return;
	pdata = self->data;

	gfc_matrix4_from_vectors(
		matrix,
		self->position,
		self->rotation,
		self->scale);
	gf3d_model_draw(
		self->model,	
		matrix,
		GFC_COLOR_GREEN,
		0);
	GFC_Vector3D* wheel;
	int i;
	GFC_Color color;
	for (i = 0, wheel = &pdata->wheelFL; i < 4; i++, wheel++) {
		if (i == 0) {
			color = gfc_color8(0, 0, 255, 63);
		}
		else {
			color = gfc_color8(255, 0, 0, 63);
		}
		gfc_matrix4_from_vectors(
			matrix,
			*wheel,
			self->rotation,
			self->scale);
		gf3d_model_draw(
			self->model,
			matrix,
			color,
			0);
	}
	//slog("ya");
	//slog(self->model);
	//slog(matrix);
	return -1;
}

Entity* spawn_player(mapData* mdata, Uint8 playerType)
{
	Entity* player = entity_new();
	playerData* pdata;
	if (!player) {
		slog("Could not create player entity");
		return 0;
	}
	pdata = gfc_allocate_array(sizeof(playerData), 1);
	if (!pdata) {
		slog("Could not allocate player data");
		return 0;
	}
	memset(pdata, 0, sizeof(playerData));
	//pdata->hitbox = gfc_sphere(player->position.x, player->position.y, player->position.z, 1.0);
	pdata->relativePos[0] = gfc_vector3d(-1, 2, 0);
	pdata->relativePos[1] = gfc_vector3d(1, 2, 0);
	pdata->relativePos[2] = gfc_vector3d(-1, -2, 0);
	pdata->relativePos[3] = gfc_vector3d(1, -2, 0);

	//player->position = gfc_vector3d(20, 0, 10);
	//player->rotation = gfc_vector3d(0, 0, 0);
	player->data = pdata;
	player->think = player_think;
	player->update = player_update;
	player->free = player_free;
	player->reset = player_reset;
	player->model = gf3d_model_load("models/primitives/sphere.obj");
		//gf3d_model_load("models/dino.model");
	player->draw = player_draw;

	pdata->mapData = mdata;
	player->position = mdata->startBlock->position;
	player->rotation = mdata->startBlock->rotation;
	player->rotation = gfc_vector3d(0,0,0);

	if (playerType == playertype_player) {
		pdata->currReplay = refresh_temp_replay(NULL);
		mapData* ghostMdata = gfc_allocate_array(sizeof(mapData), 1);
		ghostMdata->mapID = mdata->mapID;
		ghostMdata->startBlock = mdata->startBlock;
		ghostMdata->totalCheckpoints = mdata->totalCheckpoints;
		if (replay_on) {
			spawn_player(ghostMdata, playertype_replay);
		}

		mapData* aiMdata = gfc_allocate_array(sizeof(mapData), 1);
		aiMdata->mapID = mdata->mapID;
		aiMdata->startBlock = mdata->startBlock;
		aiMdata->totalCheckpoints = mdata->totalCheckpoints;
		if (ai_on) {
			spawn_player(aiMdata, playertype_ai);
		}
	}
	else if (playerType == playertype_replay) {
		pdata->currReplay = open_replay(mdata->mapID);
		if (!pdata->currReplay) {
			slog("no replay for ghost");
			player_free(player);
			return NULL;
		}
	}
	pdata->playerType = playerType;
	if (!pdata->currReplay) {
		slog("somehow NULL");
	}
	

	return player;
}