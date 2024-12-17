#include "particle.h"
#include "gfc_text.h"
#include "gf3d_pipeline.h"
#include "gf3d_buffers.h"
#include "gf3d_vgraphics.h"
#include "gf3d_mesh.h"
//#include "gfc_matrix.h"

typedef struct
{
	Particle* particleList; //big a** list
	Uint32 particleMax;
	Pipeline* pipe;
	VkBuffer  vbuffer;
	VkDeviceMemory vbuffermemory;
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

	Uint32 count;
	gf3d_mesh_get_attribute_descriptions(&count);
	particle_manager.pipe = gf3d_pipeline_create_from_config(
		gf3d_vgraphics_get_default_logical_device(),
		"config/particle_pipeline.cfg",
		gf3d_vgraphics_get_view_extent(),
		particle_manager.particleMax,
		gf3d_mesh_get_bind_description(),
		gf3d_mesh_get_attribute_descriptions(NULL),
		count,
		sizeof(MeshUBO),
		VK_INDEX_TYPE_UINT16
	);

	void* data = NULL;
	VkDevice device = gf3d_vgraphics_get_default_logical_device();
	Vertex* vertices = gfc_allocate_array(sizeof(Vertex), 3);
	Uint32 vcount = 3;
	//Face* faces;
	//Uint32 fcount;
	size_t bufferSize;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	vertices[0].vertex = gfc_vector3d(-1, -1, 0);
	vertices[1].vertex = gfc_vector3d(1, -1, 0);
	vertices[2].vertex = gfc_vector3d(0, 2, 0);

	bufferSize = sizeof(Vertex) * vcount;
	gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices, (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &particle_manager.vbuffer, &particle_manager.vbuffermemory);

	gf3d_buffer_copy(stagingBuffer, particle_manager.vbuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, NULL);
	vkFreeMemory(device, stagingBufferMemory, NULL);


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
	GFC_Matrix4 mat;
	gfc_matrix4_zero(mat);
	MeshUBO ubo = gf3d_mesh_get_ubo(mat, GFC_COLOR_WHITE);
	for (i = 0; i < particle_manager.particleMax; i++)
	{
		if (!particle_manager.particleList[i]._inuse) continue;
		gfc_matrix4_from_vectors(mat, particle_manager.particleList[i].position, particle_manager.particleList[i].rotation, gfc_vector3d(1, 1, 1));
		gfc_matrix4_copy(ubo.model, mat);
		gf3d_pipeline_queue_render(
			particle_manager.pipe,
			particle_manager.vbuffer,
			3,
			NULL,
			&ubo,
			NULL);
	}
}