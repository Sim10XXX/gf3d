#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "gfc_types.h"
#include "gfc_text.h"
#include "gfc_vector.h"
#include "gf3d_model.h"


typedef struct Entity_S
{
	Uint8			_inuse;		//flag for memory management
	GFC_TextLine	name;		//name of entity
	GFC_Vector3D	position;	//where am I
	GFC_Vector3D	rotation;	//rotation, pitch yaw roll
	GFC_Vector3D	scale;		//stretching
	Model			*model;		//graphics
	//GFC_Box			hitbox;
	//behavior
	void (*think)	(struct Entity_S *self);			//called every frame for the entity to decide things
	void (*update)	(struct Entity_S *self);			//called every frame for the entity to update its state
	int  (*draw)	(struct Entity_S *self);			//for custom drawing code. If -1, skip generic drawing code
	void (*free)	(struct Entity_S *self);			//called when the entity is cleaned up to clean up custom data
	void			*data;		//entity custom data - for everything beyond the basics
}Entity;

/**
 * @brief initialize the entity manager subsystem
 * @param maxEnts how many entities can exist at the same time
 */
void entity_system_init(Uint32 maxEnts);

void entity_update_all();

void entity_think_all();

void entity_draw_all();

void entity_check_collision(Entity* self);

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

#endif