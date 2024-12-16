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

int get_RPM() {
	return gamestate.RPM;
}

void set_RPM(int v) {
	gamestate.RPM = v;
}

int get_gear() {
	return gamestate.gear;
}

void set_gear(int v) {
	gamestate.gear = v;
}

Uint8 is_paused() {
	return gamestate.pause;
}

void set_editormode(Uint8 v) {
	gamestate.editormode = v;
}

Uint8 get_editormode() {
	return gamestate.editormode;
}
