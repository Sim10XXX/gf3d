#include "particle.h"
#include "gfc_text.h"
#include "gf3d_pipeline.h"

typedef struct
{
	Particle* particleList; //big a** list
	Uint32 particleMax;
}ParticleManager;

static ParticleManager particle_manager = { 0 };

void particle_system_close()
{
	int i;
	free(particle_manager.particleList);
	memset(&particle_manager, 0, sizeof(ParticleManager));
}

void particle_system_init(Uint32 maxParticles) {
	if (particle_manager.particleList)
	{
		return;
	}
	if (!maxParticles)
	{
		return;
	}
	particle_manager.particleList = gfc_allocate_array(sizeof(Particle), maxParticles);
	if (!particle_manager.particleList)
	{
		return;
	}
	particle_manager.particleMax = maxParticles;
	atexit(particle_system_close);
}

void create_particle(GFC_Vector3D pos, Uint8 id) {
	int i;
	for (i = 0; i < particle_manager.particleMax; i++)
	{
		if (particle_manager.particleList[i]._inuse) continue;
		particle_manager.particleList[i].position = pos;
		particle_manager.particleList[i].particleId = id;
		particle_manager.particleList[i].timeToLive = DEFAULT_TIMETOLIVE;
		particle_manager.particleList[i]._inuse = 1;
	}
}

void count_down_all_particles() {
	int i;
	for (i = 0; i < particle_manager.particleMax; i++)
	{
		if (!particle_manager.particleList[i]._inuse) continue;
		particle_manager.particleList[i].timeToLive--;
		if (particle_manager.particleList[i].timeToLive == 0) {
			memset(&particle_manager.particleList[i], 0, sizeof(Particle));
		}
	}
}


void draw_all_particles() {
	int i;
	for (i = 0; i < particle_manager.particleMax; i++)
	{
		if (!particle_manager.particleList[i]._inuse) continue;
		/*
		gf3d_pipeline_queue_render(
			pipe,
			primitive->vertexBuffer,
			primitive->faceCount * 3,
			primitive->faceBuffer,
			uboData,
			texture);*/
	}
}