#include "hud_element.h"

#include "gf2d_draw.h"
#include "gf2d_font.h"
#include "gfc_config.h"

#include "gfc_color.h"
#include "map.h"
#include "entity.h"
#include "player.h"
#include "gamestate.h"
#include "editor.h"


typedef struct
{
	Hud_Element* hudElementList; //big a** list
	Uint32 hudElementMax;
}HudManager;

static HudManager hud_manager = { 0 };

void hud_element_free(self);
void slog_all_elements();
void slog_element(Hud_Element* self);

void hud_system_close()
{
	int i;
	for (i = 0; i < hud_manager.hudElementMax; i++)
	{
		//if (!hud_manager.hudElementList[i]._inuse) continue;
		//entity_free(&hud_manager.hudElementList[i]);
	}
	free(hud_manager.hudElementList);
	memset(&hud_manager, 0, sizeof(HudManager));
}



void set_elements_state(Uint16 elementmask) {
	int i;
	for (i = 0; i < hud_manager.hudElementMax; i++)
	{
		if (!hud_manager.hudElementList[i]._inuse) continue;
		if (hud_manager.hudElementList[i].element_type & elementmask) {
			hud_manager.hudElementList[i].visible = 1;
		}
		else {
			hud_manager.hudElementList[i].visible = 0;
		}
		slog_element(&hud_manager.hudElementList[i]);
		slog("etype: %i", hud_manager.hudElementList[i].element_type);
		slog("emask: %i", elementmask);
		//slog("res: %i", hud_manager.hudElementList[i].element_type & elementmask);
	}
	if (elementmask & EDITOR_STATE) {
		set_editormode(true);
	}
	else {
		set_editormode(false);
	}
	if (elementmask & START_ELEMENT) {
		entity_free_all();
		set_pause(false);
	}
	slog_all_elements();
}

void start_map(Hud_Element *self) {
	if (!self) return;
	if (!self->filename) return;
	mapData* mdata = load_map_from_cfg(self->filename->buffer);
	if (!mdata) {
		mdata = load_empty_map(self->mapID);
	}
	if (!get_editormode()) {
		spawn_player(mdata, playertype_player);
	}
	else {
		editor_spawn(mdata);
	}
	
	set_pause(0);
	set_elements_state(GAME_ELEMENT);
	//playerData* pdata = player->data;
}

void load_map_elements();

void open_elements(Hud_Element *self, Uint16 mask) {
	if (mask & MAP_SELECT_ELEMENT) {
		load_map_elements();
	}
	set_elements_state(mask);
	//slog_all_elements();
}

void hud_system_init(const char* filename, int max_dynamic_elements)
{
	SJson* SJelements, *a;
	int maxElements;
	SJelements = sj_load(filename);
	if (!SJelements) {
		slog("Couldn't load filename");
		return 0;
	}
	SJelements = sj_object_get_value(SJelements, "elements");

	maxElements = sj_array_get_count(SJelements);

	if (hud_manager.hudElementList)
	{
		//slog("entity manager already exists");
		return;
	}
	if (!maxElements)
	{
		//slog("cannot allocate 0 entities for the entity manager");
		return;
	}
	hud_manager.hudElementList = gfc_allocate_array(sizeof(Hud_Element), max_dynamic_elements+maxElements);
	if (!hud_manager.hudElementList)
	{
		//slog("failed to allocate %i entities for the entity manager", maxElements);
		return;
	}
	hud_manager.hudElementMax = max_dynamic_elements+maxElements;
	atexit(hud_system_close);

	GFC_Vector4D vec;
	int i;
	for (i = 0; i < maxElements; i++) {
		//slog("beep: %i", i);
		a = sj_array_get_nth(SJelements, i);
		//hud_manager.hudElementList[i].data = a;
		hud_manager.hudElementList[i].visible = 1;
		sj_object_get_vector2d(a, "position", &hud_manager.hudElementList[i].position);
		GFC_Shape* s = malloc(sizeof(GFC_Shape));
		if (gfc_shape_from_json(a, s)) {
			hud_manager.hudElementList[i].shape = s;
			sj_object_get_vector4d(a, "shapecolor", &vec);
			hud_manager.hudElementList[i].shapecolor = gfc_color_from_vector4(vec);
		}
		else {
			free(s);
			hud_manager.hudElementList[i].shape = 0;
		}
		hud_manager.hudElementList[i].text = sj_object_get_gfc_string(a, "text");
		if (hud_manager.hudElementList[i].text) {
			sj_object_get_vector4d(a, "textcolor", &vec);
			hud_manager.hudElementList[i].textcolor = gfc_color_from_vector4(vec);
		}
		
		int cl;
		sj_object_get_int(a, "click", &cl);
		switch (cl) {
		case 0:
			//hud_manager.hudElementList[i].click = start_map;
			//hud_manager.hudElementList[i].filename = gfc_string("config/map.cfg");
			hud_manager.hudElementList[i].click = open_elements;
		}
		int temp;
		sj_object_get_int(a, "element_type", &temp);
		hud_manager.hudElementList[i].element_type = temp;
		sj_object_get_int(a, "set_elements", &temp);
		hud_manager.hudElementList[i].mask = temp;

		slog("elementtype: %i", hud_manager.hudElementList[i].element_type);
		hud_manager.hudElementList[i]._inuse = 1;

	}
	set_elements_state(START_ELEMENT);
}

void element_draw(Hud_Element *self) {
	if (!self) return;
	
	if (self->shape) {
		GFC_Shape* s = self->shape;
		gf2d_draw_shape(*s, self->shapecolor, self->position, 1);
	}
	if (self->text) {
		char buffer[30];
		if (self->element_type & GAME_ELEMENT) {
			Uint16 etype = self->element_type - GAME_ELEMENT;
			if (etype & GAME_ELEMENT_TIME) {
				sprintf(buffer, "Time: %.2f", get_framecount() / 30.0);
			}
			else if (etype & GAME_ELEMENT_RPM) {
				sprintf(buffer, "RPM: %i", get_RPM());
			}
			else if (etype & GAME_ELEMENT_GEAR) {
				sprintf(buffer, "Gear: %i", get_gear());
			}
			else {
				sprintf(buffer, "Speed: % i", get_speed());
			}
		}
		else {
			sprintf(buffer, gfc_string_text(self->text));
		}
		gf2d_font_draw_line_tag(buffer, FT_H1, GFC_COLOR_BLACK, gfc_vector2d(self->position.x + 1, self->position.y + 1));
		gf2d_font_draw_line_tag(buffer, FT_H1, self->textcolor, self->position);
	}
}

void draw_all_elements() {
	int i;
	for (i = 0; i < hud_manager.hudElementMax; i++)
	{
		if (!hud_manager.hudElementList[i]._inuse) continue;
		if (!is_paused()) {
			if (!hud_manager.hudElementList[i].visible) continue;
		}
		else {
			if (hud_manager.hudElementList[i].element_type != PAUSE_ELEMENT && !hud_manager.hudElementList[i].visible) continue;
		}
		element_draw(&hud_manager.hudElementList[i]);
	}
}



void check_click(int x, int y) {
	int i;
	GFC_Vector2D pos = gfc_vector2d((float) x,(float) y);
	for (i = hud_manager.hudElementMax-1; i >= 0; i--)
	{
		if (!hud_manager.hudElementList[i]._inuse) continue;
		if (!is_paused()) {
			if (!hud_manager.hudElementList[i].visible) continue;
		}
		else {
			if (hud_manager.hudElementList[i].element_type != PAUSE_ELEMENT && !hud_manager.hudElementList[i].visible) continue;
		}
		if (!hud_manager.hudElementList[i].shape) continue;
		if (!hud_manager.hudElementList[i].click) continue;
		if (gfc_point_in_shape(gfc_vector2d(pos.x- hud_manager.hudElementList[i].position.x, pos.y - hud_manager.hudElementList[i].position.y), *hud_manager.hudElementList[i].shape)) {
			//start_map("config/map.cfg");
			hud_manager.hudElementList[i].click(&hud_manager.hudElementList[i], hud_manager.hudElementList[i].mask);
		}
	}
}

void load_map_elements() {
	int i, j, hits = 1;
	SJson* sj;
	char buffer[20];
	int newid = 100;

	//first clear any map elements
	for (i = hud_manager.hudElementMax - Hud_dynamic_element_max; i < hud_manager.hudElementMax; i++)
	{
		if (!hud_manager.hudElementList[i]._inuse) continue;
		slog("nononnnnonnononononono");
		hud_element_free(&hud_manager.hudElementList[i]);
	}


	for (i = 0; i < 100 && hits < Hud_dynamic_element_max; i++) {
		sprintf(buffer, "maps/map%i.tmap", i);
		if ((sj = sj_load(buffer)) != NULL)
		{
			GFC_String *text = gfc_string(sj_object_get_string(sj, "title"));
			
			hits++;
			for (j = hud_manager.hudElementMax - Hud_dynamic_element_max; j < hud_manager.hudElementMax; j++)
			{
				if (hud_manager.hudElementList[j]._inuse) continue;
				//slog("j: %i",j);
				hud_manager.hudElementList[j]._inuse = 1;
				hud_manager.hudElementList[j].text = text;
				hud_manager.hudElementList[j].filename = gfc_string(buffer);
				hud_manager.hudElementList[j].position = gfc_vector2d(50, 50+hits*55);
				GFC_Shape* s = malloc(sizeof(GFC_Shape));
				if (s) {
					*s = gfc_shape_rect(-10, -10, 500, 50);
					hud_manager.hudElementList[j].shape = s;
				}
				hud_manager.hudElementList[j].textcolor = gfc_color8(255,255,255,255);
				hud_manager.hudElementList[j].shapecolor = gfc_color8(100, 205, 205, 255);
				hud_manager.hudElementList[j].element_type = MAP_SELECT_ELEMENT;
				hud_manager.hudElementList[j].click = start_map;
				break;


			}
			sj_free(sj);
		}
		else
		{
			if (i < newid) {
				newid = i;
			}
			//File not found, no memory leak since 'file' == NULL
			//fclose(file) would cause an error
		}
	}
	for (j = hud_manager.hudElementMax - Hud_dynamic_element_max; j < hud_manager.hudElementMax; j++)
	{
		if (hud_manager.hudElementList[j]._inuse) continue;
		//slog("j: %i",j);
		hud_manager.hudElementList[j]._inuse = 1;
		hud_manager.hudElementList[j].text = gfc_string("+ make a new map +");
		sprintf(buffer, "maps/map%i.tmap", newid);
		hud_manager.hudElementList[j].filename = gfc_string(buffer);
		hits++;
		hud_manager.hudElementList[j].position = gfc_vector2d(50, 50 + hits * 55);
		GFC_Shape* s = malloc(sizeof(GFC_Shape));
		if (s) {
			*s = gfc_shape_rect(-10, -10, 500, 50);
			hud_manager.hudElementList[j].shape = s;
		}
		hud_manager.hudElementList[j].textcolor = gfc_color8(255, 255, 255, 255);
		hud_manager.hudElementList[j].shapecolor = gfc_color8(100, 205, 205, 255);
		hud_manager.hudElementList[j].element_type = MAP_SELECT_ELEMENT;
		hud_manager.hudElementList[j].click = start_map;
		hud_manager.hudElementList[j].mapID = newid;
		break;


	}
}

void hud_element_free(Hud_Element* self) {
	if (!self) return;
	if (self->filename) gfc_string_free(self->filename);
	if (self->shape) free(self->shape);
	if (self->text) gfc_string_free(self->text);
	memset(self, 0, sizeof(Hud_Element));
}

void slog_all_elements() {
	int i;
	for (i = 0; i < hud_manager.hudElementMax; i++)
	{
		if (!hud_manager.hudElementList[i].text) continue;
		slog(hud_manager.hudElementList[i].text->buffer);
		slog("inuse: %i", hud_manager.hudElementList[i]._inuse);
	}
}
void slog_element(Hud_Element* self) {
	if (!self) {
		slog("null self");
		return;
	}
	if (!self->text) {
		slog("null text");
		return;
	}
	slog(self->text->buffer);
}