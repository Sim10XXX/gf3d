#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "simple_logger.h"
#include "gfc_types.h"
#include "gfc_vector.h"

#define DEFAULT_TIMETOLIVE 30*20
#define PARTICLE_ID_SKID 1

typedef struct
{
	GFC_Vector3D	position;
	GFC_Vector3D	rotation;
	Uint16			timeToLive;
	Uint8			particleId;
	Uint8			_inuse;
}Particle;

void particle_system_close();

void particle_system_init(Uint32 maxParticles);

void create_particle(GFC_Vector3D pos, Uint8 id);

void count_down_all_particles();

void draw_all_particles();

#endif