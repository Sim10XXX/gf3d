#include "gamestate.h"

Gamestate gamestate = { 0 };

void toggle_pause() {
	gamestate.pause = !gamestate.pause;
}

void set_pause(Uint8 v) {
	gamestate.pause = v;
}

int get_framecount() {
	return gamestate.framecount;
}

void set_framecount(int v) {
	gamestate.framecount = v;
}

int get_speed() {
	return gamestate.speed;
}

void set_speed(int v) {
	gamestate.speed = v;
}

Uint8 is_paused() {
	return gamestate.pause;
}