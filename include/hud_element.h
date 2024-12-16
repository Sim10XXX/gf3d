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
	Uint16			element_type;	//type of hud element
	//SJson			*data; // contains text, backdrop shape, sprite. Basically the draw information
	Uint8			visible; // for drawing and for clicking logic
	Uint8			_inuse;
	Uint16			mask; //Used for navigation elements
	GFC_String		*filename;
	int				mapID; //Used for creating new maps
	void (*click)	(struct Hud_Element_S* self, Uint8 mask);
	void (*hover)	(struct Hud_Element_S* self);
	//void (*free)	(struct Hud_Element_S* self);
}Hud_Element;

#define Hud_dynamic_element_max 32

#define PAUSE_ELEMENT 1
#define GAME_ELEMENT 2
#define GAME_ELEMENT_TIME 2+128
#define GAME_ELEMENT_RPM 2+512
#define GAME_ELEMENT_GEAR 2+256
#define START_ELEMENT 4
#define MAP_SELECT_ELEMENT 8
#define EDITOR_STATE 16

void hud_system_init(const char* filename, int max_dynamic_elements);

void set_elements_state(Uint16 elementmask);

void draw_all_elements();

void check_click(int x, int y);

#endif