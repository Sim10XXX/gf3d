#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "gfc_types.h"
#include "gfc_text.h"
#include "gfc_vector.h"
#include "gf3d_model.h"

#define collisions_max 8

#define SURFACE_ROAD 0
#define SURFACE_P_GRASS 1
#define SURFACE_DIRT 2
#define SURFACE_ICE 4
#define SURFACE_WOOD 8

typedef struct Entity_S
{
	Uint8			_inuse;		//flag for memory management
	Uint8			colliding;
	//GFC_TextLine	name;		//name of entity
	GFC_Vector3D	position;	//where am I
	GFC_Vector3D	rotation;	//rotation, pitch yaw roll
	GFC_Vector3D	scale;		//stretching
	Model			*model;		//graphics
	Uint8			isBlock;
	GFC_Color		colormod;
	float			collisionRadius; //don't even check collision outside of this radius, 0 means disabled (always check)
	//GFC_Box			hitbox;
	//behavior
	void (*think)	(struct Entity_S *self);			//called every frame for the entity to decide things
	void (*update)	(struct Entity_S *self);			//called every frame for the entity to update its state
	int  (*draw)	(struct Entity_S *self);			//for custom drawing code. If -1, skip generic drawing code
	void (*free)	(struct Entity_S *self);			//called when the entity is cleaned up to clean up custom data
	void (*touch)	(struct Entity_S* self, struct Entity_S* other);
	void (*reset)	(struct Entity_S* self);
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

void entity_reset(); //reset things like checkpoints when restarting a map


/**
* Check what faces a sphere (wheel) is colliding with, and populate the list with relevant vector perpendicular to that face,
* where the magnitude happens to be equal to the sphere's radius
*/
void check_player_collision(Entity* self, GFC_Sphere s, GFC_Vector3D* vlist, GFC_Vector3D *mlist);

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

void entity_free_all();

Entity* get_next_block();

Entity* get_closest_block(Uint8 checkpointorfinish, float maxrange, GFC_Vector3D position);

void delete_blocks_in_range(GFC_Vector3D minposition, GFC_Vector3D maxposition);

void delete_duplicate_blocks();

#endif