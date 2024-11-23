#ifndef __GAMESTATE_H__
#define __GAMESTATE_H__

#include "gfc_types.h"

typedef struct {
	Uint8 pause;
	int framecount;
	int speed;
} Gamestate;

void toggle_pause();

void set_pause(Uint8 v);

int get_framecount();

void set_framecount(int v);

int get_speed();

void set_speed(int v);

Uint8 is_paused();


#endif