#ifndef __REPLAY_H__
#define __REPLAY_H__

#include "simple_logger.h"
#include "player.h"

FILE* open_replay(int id);

int get_replay_size(FILE* file);

int read_replay(FILE* file);

FILE* refresh_temp_replay(FILE* file);

void append_to_temp_replay(int inputs, FILE* file);

void save_temp_replay(int id, FILE* file);


#endif