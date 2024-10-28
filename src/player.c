#include "simple_logger.h"

#include "gfc_matrix.h"
#include "gf3d_camera.h"
#include "gfc_input.h"
#include "entity.h"
#include "player.h"
#include "force.h"
#include "replay.h"

#define friction 0.02
#define wheel_radius 1
#define normal_force_mult 0.59


void project(Entity* self, playerData* pdata, Entity* projectionEnt, playerData* projectionData);

int player_get_inputs(playerData* pdata) {
	int inputs = 0;
	if (pdata->isGhost) {
		inputs = read_replay(pdata->currReplay);
		//slog("inputs: %i", inputs);
		return inputs;
	}
	
	if (gfc_input_command_down("respawn")) {
		inputs |= key_respawn;
	}
	if (gfc_input_command_down("walkforward")) {
		inputs |= key_up;
	}
	if (gfc_input_command_down("walkbackward")) {
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

	self->position = mdata->startBlock->position;
	self->rotation = mdata->startBlock->rotation;
	
	
}


void player_think(Entity* self) 
{
	if (!self)return;
	playerData* pdata = self->data;
	if (!pdata)return;
	mapData* mdata = pdata->mapData;
	if (!mdata)return;

	GFC_Vector3D dv = {0};

	if (!pdata->gameState) {
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

	
	if (pdata->framecount == 1) { //skip first frame because otherwise bad things happen
		return;
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

		if (pdata->isGhost) {
			if (pdata->currReplay) {
				fclose(pdata->currReplay);
				pdata->currReplay = open_replay(mdata->mapID);
			}
			
		}
		else {
			pdata->currReplay = refresh_temp_replay(pdata->currReplay);
		}
		

		return;
	}
	int inputs = 0;
	if (!pdata->gameState) {
		inputs = player_get_inputs(pdata);
		if (!pdata->isGhost) {
			append_to_temp_replay(inputs, pdata->currReplay);
		}
	}
	

	if ((key_respawn & inputs) && !pdata->gameState) { //could remove the checks for gamestate
		Entity* checkpointBlock = mdata->lastCheckpoint;
		player_reset(self);
		self->position = checkpointBlock->position;
		self->rotation = checkpointBlock->rotation;
		return;
	}

	if ((key_up & inputs) && !pdata->gameState) {
		//pdata->positionVelocity.y += 0.01;

		gfc_vector2d_scale(dv, gfc_vector2d_from_angle(self->rotation.z),0.05);
		//gfc_vector2d_add(pdata->positionVelocity, pdata->positionVelocity, dv);
		//slog("vx= %f", pdata->positionVelocity.x);
		apply_force(
			force3d(gfc_vector3d(0, 0, 0),
				gfc_vector3d(dv.x, dv.y, 0)), //sin(self->rotation.x)*0.05)
			self, 0);
	}
	if ((key_right & inputs) && !pdata->gameState) {
		pdata->rotationVelocity.z -= 0.005;
		apply_force(
			force3d(gfc_vector3d(0, radius_of_player, 0),
				gfc_vector3d(-0.01, 0, 0)),
			self, 0);
	}
		GFC_Vector3D forcevec, forwardvec, upvec, velocity;
		velocity = pdata->positionVelocity;
		
		gfc_vector3d_sub(forwardvec, pdata->wheelRR, pdata->wheelFR);
		gfc_vector3d_sub(upvec, pdata->aboveFR, pdata->wheelFR);
		gfc_vector3d_set_magnitude(&forwardvec, 1);
		gfc_vector3d_set_magnitude(&upvec, 1);
		//gfc_vector3d_add(v2, pdata->wheelFR, gfc_vector3d(0, 0, 5));

		//slog("x: %f, y: %f, z: %f", gfc_vector3d_to_slog(velocity));
		//slog("x: %f, y: %f, z: %f", gfc_vector3d_to_slog(forwardvec));
		//slog("dot: %f",gfc_vector3d_dot_product(forwardvec, velocity));

		//gfc_vector3d_cross_product(&forcevec, forwardvec, upvec);
		//gfc_vector3d_set_magnitude(&forcevec, gfc_vector3d_dot_product(forwardvec, velocity)*0.1);

		//forcevec = gfc_vector3d(0.01, 0, 0);
		//gfc_vector3d_set_magnitude(&forcevec, gfc_vector3d_dot_product(forwardvec, velocity) * -0.01);

		forwardvec = pdata->forward;
		gfc_vector3d_set_magnitude(&velocity, gfc_vector3d_magnitude(forwardvec));
		gfc_vector3d_sub(forcevec, forwardvec, velocity);
		gfc_vector3d_set_magnitude(&forcevec, 0.01);

		//apply_force(
		//	force3d(gfc_vector3d(0,0,0),//pdata->relativePos[1],
		//		forcevec), //forcevec
		//	self, 0);
	
	if ((key_left & inputs) && !pdata->gameState) {
		pdata->rotationVelocity.z += 0.005;
		apply_force(
			force3d(gfc_vector3d(0, radius_of_player, 0),
				gfc_vector3d(0.01, 0, 0)),
			self, 0);
	}

	//entity_check_collision(self);

	//Add gravity force
	//force.origin = gfc_vector3d(1, 1, 0);
	//force.forceVector = gfc_vector3d(0, 0, -0.001);

	apply_force(
		force3d(gfc_vector3d(0, 0, 0), 
			gfc_vector3d(0, 0, -0.02)),
		self, 0);
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
	for (i = 0; i < 4 * collisions_max; i++) {
		apply_force(forceQueue[i], self, 0);
		//slog("forcev x: %f, y: %f, z: %f", gfc_vector3d_to_slog(forceQueue[i].forceVector));
	}

	
	/*gfc_vector3d_sub(finalv, pdata->wheelFL, self->position);
	gfc_vector3d_cross_product(&finalv, pdata->rotationVelocity, finalv);
	gfc_vector3d_add(finalv, finalv, pdata->positionVelocity);
	slog("WheelFL x: %f, y: %f, z: %f",finalv.x, finalv.y, finalv.z);*/
	
	
	//slog("projectiondata x: %f, y: %f, z: %f", projectionData.wheelFL.x, projectionData.wheelFL.y, projectionData.wheelFL.z);
	//slog("pdata x: %f, y: %f, z: %f", pdata->wheelFL.x, pdata->wheelFL.y, pdata->wheelFL.z);
	//slog("WheelFL x: %f, y: %f, z: %f", finalv[0].x, finalv[0].y, finalv[0].z);
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

	gfc_vector3d_scale(deltaPV, pdata->positionVelocity, friction);
	gfc_vector3d_scale(deltaRV, pdata->rotationVelocity, friction);

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
	gfc_vector3d_scale(fvec, pdata->positionVelocity, friction);
	if (gfc_vector3d_magnitude(fvec) > 0.01) {
		gfc_vector3d_set_magnitude(&fvec, 0.01);
	}
	gfc_vector3d_sub(pdata->positionVelocity, pdata->positionVelocity, fvec);

	gfc_vector3d_scale(fvec, pdata->rotationVelocity, friction);
	if (gfc_vector3d_magnitude(fvec) > 0.01) {
		gfc_vector3d_set_magnitude(&fvec, 0.01);
	}
	gfc_vector3d_sub(pdata->rotationVelocity, pdata->rotationVelocity, fvec);

}

void player_update(Entity* self)
{
	GFC_Vector3D lookTarget, camera, dir = { 0 };
	playerData* pdata;
	if (!self) return;
	if (!self->data)return;
	pdata = self->data;

	move_player(self);
	//velocity_update(self);
	//update_hitbox(self);
	apply_friction(self);
	
	

	gfc_vector3d_copy(lookTarget, self->position);
	lookTarget.z += 1;
	dir.y = 30.0;

	if (!pdata->isGhost) {
		gfc_vector3d_rotate_about_z(&dir, self->rotation.z);
		gfc_vector3d_sub(camera, self->position, dir);
		camera.z += 10;
		gf3d_camera_look_at(lookTarget, &camera);
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

Entity* spawn_player(mapData* mdata, Uint8 isGhost)
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

	if (!isGhost) {
		pdata->currReplay = refresh_temp_replay(NULL);
		mapData* ghostMdata = gfc_allocate_array(sizeof(mapData), 1);
		ghostMdata->mapID = mdata->mapID;
		ghostMdata->startBlock = mdata->startBlock;
		ghostMdata->totalCheckpoints = mdata->totalCheckpoints;
		spawn_player(ghostMdata, 1);
	}
	else {
		pdata->currReplay = open_replay(mdata->mapID);
		if (!pdata->currReplay) {
			slog("no replay for ghost");
			player_free(player);
			return NULL;
		}
	}
	pdata->isGhost = isGhost;
	if (!pdata->currReplay) {
		slog("somehow NULL");
	}
	

	return player;
}