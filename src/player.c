#include "simple_logger.h"

#include "gfc_matrix.h"
#include "gf3d_camera.h"
#include "gfc_input.h"
#include "entity.h"
#include "player.h"
#include "force.h"

#define friction 0.02
#define wheel_radius 1
#define normal_force_mult 0.59


void project(Entity* self, playerData* pdata, Entity* projectionEnt, playerData* projectionData);

void player_free(Entity* self) 
{
	if (!self) {
		slog("invalid self pointer for player_free");
		return;
	}
	if (!self->data) {
		slog("no player data to free");
		return;
	}
	free(self->data);

}

void player_think(Entity* self) 
{
	if (!self)return;
	if (!self->data)return;
	playerData* pdata = self->data;
	GFC_Vector3D dv = {0};

	if (pdata->framecount == 0) {
		pdata->framecount++;
		return;
	}

	if (gfc_input_command_down("walkforward")) {
		//pdata->positionVelocity.y += 0.01;

		gfc_vector2d_scale(dv, gfc_vector2d_from_angle(self->rotation.z),0.05);
		//gfc_vector2d_add(pdata->positionVelocity, pdata->positionVelocity, dv);
		//slog("vx= %f", pdata->positionVelocity.x);
		apply_force(
			force3d(gfc_vector3d(0, 0, 0),
				gfc_vector3d(dv.x, dv.y, 0)),
			pdata);
	}
	if (gfc_input_command_down("walkright")) {
		//pdata->rotationVelocity.z += 0.01;
		apply_force(
			force3d(gfc_vector3d(0, radius_of_player, 0),
				gfc_vector3d(-0.01, 0, 0)),
			pdata);
	}
	if (gfc_input_command_down("walkleft")) {
		//pdata->rotationVelocity.z -= 0.01;
		apply_force(
			force3d(gfc_vector3d(0, radius_of_player, 0),
				gfc_vector3d(0.01, 0, 0)),
			pdata);
	}

	//entity_check_collision(self);

	//Add gravity force
	//force.origin = gfc_vector3d(1, 1, 0);
	//force.forceVector = gfc_vector3d(0, 0, -0.001);

	apply_force(
		force3d(gfc_vector3d(0, 0, 0), 
			gfc_vector3d(0, 0, -0.01)),
		pdata);
	//pdata->rotationVelocity.x = 0.01;
	GFC_Vector3D* wheel;
	int i, j, k, l;
	int equalcount;
	//GFC_Vector3D vlist[collisions_max];
	GFC_Vector3D vlistlist[4][collisions_max];
	memset(&vlistlist, 0, sizeof(GFC_Vector3D) * collisions_max * 4);

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



	for (i = 0, wheel = &pdata->wheelFL; i < 4; i++, wheel++) {
		check_player_collision(gfc_sphere(wheel->x, wheel->y, wheel->z, wheel_radius),&vlistlist[i]);
	}
	for (i = 0; i < 4; i++) {
		for (j = 0; j < collisions_max; j++) {
			if (vlistlist[i][j].x == 0 && vlistlist[i][j].y == 0 && vlistlist[i][j].z == 0) continue;
			//gfc_vector3d_set_magnitude(&vlist[j], 0.001);

			//Since the force from vlist is supposed to be the direction of the normal force,
			//We need to calculate the magnitude by using the player's current velocity
			float mag = gfc_vector3d_dot_product(finalv[i], vlistlist[i][j]) / gfc_vector3d_magnitude(vlistlist[i][j]);
			mag *= -normal_force_mult;
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
				pdata);*/
			forceQueue[forceQueueC] = force3d(pdata->relativePos[i],vlistlist[i][j]);
			forceQueueC++;
			slog("hit #%i", forceQueueC);
		}
	}
	for (i = 0; i < 4 * collisions_max; i++) {
		apply_force(forceQueue[i], pdata);
	}

	
	/*gfc_vector3d_sub(finalv, pdata->wheelFL, self->position);
	gfc_vector3d_cross_product(&finalv, pdata->rotationVelocity, finalv);
	gfc_vector3d_add(finalv, finalv, pdata->positionVelocity);
	slog("WheelFL x: %f, y: %f, z: %f",finalv.x, finalv.y, finalv.z);*/
	
	
	//slog("projectiondata x: %f, y: %f, z: %f", projectionData.wheelFL.x, projectionData.wheelFL.y, projectionData.wheelFL.z);
	//slog("pdata x: %f, y: %f, z: %f", pdata->wheelFL.x, pdata->wheelFL.y, pdata->wheelFL.z);
	slog("WheelFL x: %f, y: %f, z: %f", finalv[0].x, finalv[0].y, finalv[0].z);
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
	

	gfc_vector3d_copy(lookTarget, self->position);
	lookTarget.z += 1;
	dir.y = 30.0;


	gfc_vector3d_rotate_about_z(&dir, self->rotation.z);
	gfc_vector3d_sub(camera, self->position, dir);
	camera.z += 10;
	gf3d_camera_look_at(lookTarget, &camera);

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
	for (i = 0, wheel = &pdata->wheelFL; i < 4; i++, wheel++) {
		gfc_matrix4_from_vectors(
			playerMatrix,
			self->position,
			self->rotation,
			self->scale);
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
	}
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

Entity* spawn_player()
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

	player->position = gfc_vector3d(0, 0, 10);
	player->data = pdata;
	player->think = player_think;
	player->update = player_update;
	player->free = player_free;
	player->model = gf3d_model_load("models/primitives/sphere.obj");
		//gf3d_model_load("models/dino.model");
	player->draw = player_draw;
	return player;
}