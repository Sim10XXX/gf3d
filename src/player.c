#include "simple_logger.h"

#include "gfc_matrix.h"
#include "gf3d_camera.h"
#include "gfc_input.h"
#include "entity.h"

#define friction 0.02

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
}playerData;




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
	if (gfc_input_command_down("walkforward")) {
		//pdata->positionVelocity.y += 0.01;

		gfc_vector2d_scale(dv, gfc_vector2d_from_angle(self->rotation.z),0.05);
		gfc_vector2d_add(pdata->positionVelocity, pdata->positionVelocity, dv);
		//slog("vx= %f", pdata->positionVelocity.x);
	}
	if (gfc_input_command_down("walkright")) {
		pdata->rotationVelocity.z += 0.01;
	}
	if (gfc_input_command_down("walkleft")) {
		pdata->rotationVelocity.z -= 0.01;
	}

	//entity_check_collision(self);

}
//slog("position %f %f %f", self->position.x, self->position.y, self->position.z)


void move_player(Entity* self) 
{
	if (!self)return;
	if (!self->data)return;
	playerData* pdata = self->data;
	gfc_vector3d_add(self->position, pdata->positionVelocity, self->position);
	gfc_vector3d_add(self->rotation, pdata->rotationVelocity, self->rotation);
	
	//self->hitbox = gfc_box(self->position.x - 2, self->position.y - 2, self->position.z - 2, 4, 4, 4);

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
	velocity_update(self);
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
	GFC_Vector3D relativePos[4] = {
		gfc_vector3d(-1, 2, 0),
		gfc_vector3d(1, 2, 0),
		gfc_vector3d(-1, -2, 0),
		gfc_vector3d(1, -2, 0) };

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
			relativePos[i],
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
	for (i = 0, wheel = &pdata->wheelFL; i < 4; i++, wheel++) {
		gfc_matrix4_from_vectors(
			matrix,
			*wheel,
			self->rotation,
			self->scale);
		gf3d_model_draw(
			self->model,
			matrix,
			gfc_color8(255,0,0,63),
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
	player->data = pdata;
	player->think = player_think;
	player->update = player_update;
	player->free = player_free;
	player->model = gf3d_model_load("models/primitives/sphere.obj");
		//gf3d_model_load("models/dino.model");
	player->draw = player_draw;
	return player;
}