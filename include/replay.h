#ifndef __REPLAY_H__
#define __REPLAY_H__

#include "simple_logger.h"
#include "player.h"

void read_replay(int id, int frame);

void refresh_temp_replay();

void append_to_temp_replay(int frame, int inputs);

void save_temp_replay(int id);


#endif