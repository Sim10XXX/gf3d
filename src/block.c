#include "simple_logger.h"

//#include "gfc_matrix.h"

#include "entity.h"

typedef struct
{
	float padding;
	GFC_Triangle3D hitbox;
}blockData;

void block_free(Entity* self) {
	if (!self) {
		slog("invalid self pointer for block_free");
		return;
	}
	if (!self->data) {
		slog("no block data to free");
		return;
	}
	free(self->data);
}

Entity* spawn_block() {
	Entity* block = entity_new();
	blockData* bdata;
	if (!block) {
		slog("Could not create block entity");
		return 0;
	}
	bdata = gfc_allocate_array(sizeof(blockData), 1);
	if (!bdata) {
		slog("Could not allocate block data");
		return 0;
	}
	//memset(bdata, 0, sizeof(blockData));

	block->position.z = -2;

	//block->hitbox = gfc_box(block->position.x - 2, block->position.y - 2, block->position.z - 0.1, 4, 4, 0.2);
	GFC_Vector3D p1, p2, p3;
	gfc_vector3d_add(p1, block->position,gfc_vector3d(-2.0,-2.0,0.1));
	gfc_vector3d_add(p2, block->position, gfc_vector3d(2.0, -2.0, 0.1));
	gfc_vector3d_add(p3, block->position, gfc_vector3d(-2.0, 2.0, 0.1));

	bdata->hitbox = gfc_triangle(p1,p2,p3);
	block->data = bdata;
	block->scale = gfc_vector3d(4, 4, 0.2);
	block->model = gf3d_model_load("models/platform.model");
		//gf3d_model_load("models/primitives/cube.obj");
	block->free = block_free;
}