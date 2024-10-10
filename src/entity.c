#include "simple_logger.h"

#include "gfc_matrix.h"

#include "entity.h"


typedef struct
{
	Entity *entityList; //big a** list
	Uint32 entityMax;
}EntityManager;

static EntityManager entity_manager = {0};

void entity_system_close()
{
	int i;
	for (i = 0; i < entity_manager.entityMax; i++)
	{
		if (!entity_manager.entityList[i]._inuse) continue;
		entity_free(&entity_manager.entityList[i]);
	}
	free(entity_manager.entityList);
	memset(&entity_manager, 0, sizeof(EntityManager));
}

void entity_system_init(Uint32 maxEnts)
{
	if (entity_manager.entityList)
	{
		slog("entity manager already exists");
		return;
	}
	if (!maxEnts)
	{
		slog("cannot allocate 0 entities for the entity manager");
		return;
	}
	entity_manager.entityList = gfc_allocate_array(sizeof(Entity), maxEnts);
	if (!entity_manager.entityList)
	{
		slog("failed to allocate %i entities for the entity manager", maxEnts);
		return;
	}
	entity_manager.entityMax = maxEnts;
	atexit(entity_system_close);
}

void entity_draw(Entity *self) 
{
	GFC_Matrix4 matrix;
	if (!self) return;

	if (self->draw)
	{
		if (self->draw(self) == -1) return;
	}
	
	gfc_matrix4_from_vectors(
		matrix,
		self->position,
		self->rotation,
		self->scale);
	gf3d_model_draw(
		self->model,
		matrix,
		GFC_COLOR_WHITE,
		0);
	//slog("ya");
	//slog(self->model);
	//slog(matrix);
}

void entity_draw_all()
{
	int i;
	for (i = 0; i < entity_manager.entityMax; i++)
	{
		if (!entity_manager.entityList[i]._inuse) continue; //
		entity_draw(&entity_manager.entityList[i]);
	}
}

void entity_think(Entity *self)
{
	if (!self) return;
	if (self->think) self->think(self);
}

void entity_think_all()
{
	int i;
	for (i = 0; i < entity_manager.entityMax; i++)
	{
		if (!entity_manager.entityList[i]._inuse) continue; //
		//slog("entity #%i",i);
		entity_think(&entity_manager.entityList[i]);
	}
}

void entity_update(Entity* self)
{
	if (!self)return;
	if (self->update) self->update(self);
}

void entity_update_all()
{
	int i;
	for (i = 0; i < entity_manager.entityMax; i++)
	{
		if (!entity_manager.entityList[i]._inuse)continue; //
		entity_update(&entity_manager.entityList[i]);
	}
}

/*
void entity_check_collision(Entity* self)
{
	if (!self)return;
	int i;
	for (i = 0; i < entity_manager.entityMax; i++)
	{
		if (!entity_manager.entityList[i]._inuse)continue; //
		if (entity_manager.entityList + i == self) continue;
		if (gfc_box_overlap(self->hitbox, entity_manager.entityList[i].hitbox)) {
			slog("WE GOT A HIT");
		}
	}
}*/

Entity *entity_new()
{
	int i;
	for (i = 0; i < entity_manager.entityMax; i++)
	{
		if (entity_manager.entityList[i]._inuse) continue; //skip ones in use
		memset(&entity_manager.entityList[i], 0, sizeof(Entity)); //clear it out in case anything was still there
		entity_manager.entityList[i]._inuse = 1;
		entity_manager.entityList[i].scale = gfc_vector3d(1, 1, 1);

		//any default values should be set
		return &entity_manager.entityList[i];
	}
	return NULL; //no more entity slots
}

void entity_free(Entity* self)
{
	if (!self) return;
	//free up anything that was allocated FOR this
	if (self->free) self->free(self);

	gf3d_model_free(self->model);
	memset(self, 0, sizeof(Entity));
}

/**
 * @brief allocates a blank entity for use
 * @returns NULL on failure (out of memory) or a pointer to the initialized entity
 */
Entity* entity_new();

/**
 * @brief return the memory of a previously allocated entity back to the pool
 * @param self the entity to free
 */
void entity_free(Entity* self);

/*eol@eof*/