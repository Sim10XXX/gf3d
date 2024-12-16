#ifndef __GAMESTATE_H__
#define __GAMESTATE_H__

#include "gfc_types.h"

typedef struct {
	Uint8 pause;
	int framecount;
	int speed;
	int RPM;
	int gear;
	Uint8 editormode;
} Gamestate;

void toggle_pause();

void set_pause(Uint8 v);

int get_framecount();

void set_framecount(int v);

int get_speed();

void set_speed(int v);

int get_RPM();

void set_RPM(int v);

int get_gear();

void set_gear(int v);



Uint8 is_paused();

void set_editormode(Uint8 v);

Uint8 get_editormode();


#endif