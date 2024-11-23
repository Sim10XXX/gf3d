#ifndef __HUD_ELEMENT_H__
#define __HUD_ELEMENT_H__

#include "simple_logger.h"
#include "gfc_types.h"
#include "gfc_text.h"
#include "simple_json.h"
#include "gfc_string.h"
#include "gfc_shape.h"

typedef struct Hud_Element_S
{
	GFC_Vector2D	position; 
	GFC_String		*text;
	GFC_Shape		*shape;
	GFC_Color		textcolor;
	GFC_Color		shapecolor;
	Uint8			element_type;	//type of hud element
	//SJson			*data; // contains text, backdrop shape, sprite. Basically the draw information
	Uint8			visible; // for drawing and for clicking logic
	Uint8			_inuse;
	GFC_String		*filename;
	void (*click)	(struct Hud_Element_S* self);
	void (*hover)	(struct Hud_Element_S* self);
	//void (*free)	(struct Hud_Element_S* self);
}Hud_Element;

#define Hud_dynamic_element_max 32

#define PAUSE_ELEMENT 1
#define GAME_ELEMENT 2
#define GAME_ELEMENT_TIME 2+128
#define START_ELEMENT 4
#define MAP_SELECT_ELEMENT 8

void hud_system_init(const char* filename, int max_dynamic_elements);

void draw_all_elements();

void check_click(int x, int y);

#endif