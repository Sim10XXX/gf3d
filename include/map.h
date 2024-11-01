#ifndef __MAP_H__
#define __MAP_H__

#include "simple_logger.h"
#include "entity.h"

typedef struct
{
	Uint8 totalCheckpoints;
	Uint8 currentCheckpoints;
	Entity* lastCheckpoint;
	Entity* startBlock;
	int mapID;
	Uint8 hasNodes;
}mapData;

mapData* load_map_from_cfg(const char* filename);

#endif