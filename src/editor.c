#include "simple_logger.h"

#include "gfc_matrix.h"
#include "gf3d_camera.h"
#include "gfc_input.h"
//#include "node.h"
#include "gamestate.h"
#include "block.h"

#include "editor.h"
#include "gf3d_draw.h"

#define CYCLE_MIN 1
#define CYCLE_MAX MAX_ID

void editor_select_new_block(Entity* self) {
	if (!self) return;
	editorData* edata = self->data;
	if (!edata) return;

	edata->selectedBlock = spawn_block(edata->currcycle);
	edata->selectedBlock->colormod = GFC_COLOR_LIGHTGREEN;
}

void editor_think(Entity* self) {
	if (!self) return;
	editorData* edata = self->data;
	if (!edata) return;
	int cycle = 0;

	mapData* mdata = edata->mdata;
	if (!mdata) return;


	if (gfc_input_command_pressed("cycleright")) {
		cycle = 1;
	}
	if (gfc_input_command_pressed("cycleleft")) {
		cycle = -1;
	}
	if (cycle && !edata->nodeMode) {
		edata->currcycle += cycle;
		if (edata->currcycle < CYCLE_MIN) {
			edata->currcycle = CYCLE_MAX;
		}
		else if (edata->currcycle > CYCLE_MAX) {
			edata->currcycle = CYCLE_MIN;
		}
		if (edata->selectedBlock) {
			entity_free(edata->selectedBlock);
		}

		edata->selectedBlock = spawn_block(edata->currcycle);
		edata->selectedBlock->colormod = GFC_COLOR_LIGHTGREEN;
	}
	float yawdelta = 0, pitchdelta = 0;

	float deltamult = 1;
	if (gfc_input_command_down("editorfinemovement")) {
		deltamult = 0.1;
	}

	if (gfc_input_command_down("pandown")) {
		pitchdelta -= GFC_PI * 0.01;
	}
	if (gfc_input_command_down("panup")) {
		pitchdelta += GFC_PI * 0.01;
	}
	if (gfc_input_command_down("panleft")) {
		yawdelta -= GFC_PI * 0.02;
	}
	if (gfc_input_command_down("panright")) {
		yawdelta += GFC_PI * 0.02;
	}
	if (edata->cameraPitch + pitchdelta > -GFC_PI && edata->cameraPitch + pitchdelta < 0) {
		edata->cameraPitch += pitchdelta;
	}

	float distancedelta = 0;
	if (gfc_input_command_down("zoomin")) {
		distancedelta = +5 * deltamult;
	}
	if (gfc_input_command_down("zoomout")) {
		distancedelta = -5 * deltamult;
	}
	if (edata->cameraDistance + distancedelta > 5) {
		edata->cameraDistance += distancedelta;
	}
	
	edata->cameraYaw += yawdelta;
	gfc_angle_clamp_radians(&edata->cameraYaw);

	float *forward, *leftright;
	int fdir, lrdir;
	if (edata->cameraYaw < GFC_HALF_PI * 0.5 || edata->cameraYaw >= GFC_HALF_PI * 3.5) {
		forward = &self->position.y;
		leftright = &self->position.x;
		fdir = -1;
		lrdir = -1;
	}
	else if (edata->cameraYaw >= GFC_HALF_PI * 0.5 && edata->cameraYaw < GFC_HALF_PI * 1.5) {
		forward = &self->position.x;
		leftright = &self->position.y;
		fdir = 1;
		lrdir = -1;
	}
	else if (edata->cameraYaw >= GFC_HALF_PI * 1.5 && edata->cameraYaw < GFC_HALF_PI * 2.5) {
		forward = &self->position.y;
		leftright = &self->position.x;
		fdir = 1;
		lrdir = 1;
	}
	else { //if (edata->cameraYaw >= GFC_HALF_PI * 2.5 && edata->cameraYaw < GFC_HALF_PI * 3.5) {
		forward = &self->position.x;
		leftright = &self->position.y;
		fdir = -1;
		lrdir = 1;
	}
	if (gfc_input_command_pressed("walkforward")) {
		*forward = *forward + 20 * fdir * deltamult;
	}
	if (gfc_input_command_pressed("walkback")) {
		*forward = *forward - 20 * fdir * deltamult;
	}
	if (gfc_input_command_pressed("walkleft")) {
		*leftright = *leftright - 20 * lrdir * deltamult;
	}
	if (gfc_input_command_pressed("walkright")) {
		*leftright = *leftright + 20 * lrdir * deltamult;
	}
	if (gfc_input_command_pressed("editorup")) {
		self->position.z += 20 * deltamult;
	}
	if (gfc_input_command_pressed("editordown")) {
		self->position.z -= 20 * deltamult;
	}
	if (gfc_input_command_pressed("editorplace")) {
		if (!edata->nodeMode) {
			edata->selectedBlock->colormod = GFC_COLOR_WHITE;
			if (bdata_get_start(edata->selectedBlock)) edata->selectedBlock->colormod = GFC_COLOR_RED;
			editor_select_new_block(self);
		}
		else {
			edata->selectedNode = spawn_node(self->position);
		}
	}
	if (gfc_input_command_pressed("editordelete")) {
		if (!edata->nodeMode) {
			delete_blocks_in_range(gfc_vector3d(self->position.x - 9.9, self->position.y - 9.9, self->position.z - 9.9),
				gfc_vector3d(self->position.x + 9.9, self->position.y + 9.9, self->position.z + 9.9));
		}
		else {
			delete_near_nodes(self->position, 5);
		}
		
	}
	if (gfc_input_command_pressed("editorrotatex")) {
		self->rotation.x += GFC_HALF_PI * deltamult;
	}
	if (gfc_input_command_pressed("editorrotatey")) {
		self->rotation.y += GFC_HALF_PI * deltamult;
	}
	if (gfc_input_command_pressed("editorrotatez")) {
		self->rotation.z += GFC_HALF_PI * deltamult;
	}
	if (gfc_input_command_pressed("editorrotatexminus")) {
		self->rotation.x -= GFC_HALF_PI * deltamult;
	}
	if (gfc_input_command_pressed("editorrotateyminus")) {
		self->rotation.y -= GFC_HALF_PI * deltamult;
	}
	if (gfc_input_command_pressed("editorrotatezminus")) {
		self->rotation.z -= GFC_HALF_PI * deltamult;
	}
	if (gfc_input_command_pressed("editornodemode")) {
		if (edata->nodeMode) {
			edata->nodeMode = 0;
			editor_select_new_block(self);
		}
		else {
			edata->nodeMode = 1;
			if (edata->selectedBlock) {
				entity_free(edata->selectedBlock);
			}
			//edata->selectedNode = spawn_node(self->position);
		}
	}

	if (gfc_input_command_pressed("editorsave")) {
		entity_free(edata->selectedBlock);
		mdata->mapID = convert_current_entities_into_map(mdata->mapID);
		editor_select_new_block(self);
	}
	if (gfc_input_command_pressed("editortogglestart")) {
		Uint8 t = bdata_get_start(edata->selectedBlock);
		bdata_set_start(edata->selectedBlock, !t);
		if (bdata_get_start(edata->selectedBlock)) {
			edata->selectedBlock->colormod = GFC_COLOR_LIGHTRED;
		}
		else {
			edata->selectedBlock->colormod = GFC_COLOR_LIGHTGREEN;
		}
	}
}



void editor_update(Entity* self){
	if (!self) return;
	editorData *edata = self->data;
	if (!edata) return;
	GFC_Vector3D cpos;

	GFC_Vector2D yaw_vec = gfc_vector2d_from_angle(edata->cameraYaw);
	GFC_Vector2D pitch_vec = gfc_vector2d_from_angle(edata->cameraPitch);

	gfc_vector2d_set_magnitude(&yaw_vec, pitch_vec.x);

	cpos.x = yaw_vec.x;
	cpos.y = yaw_vec.y;
	cpos.z = pitch_vec.y;
	//slog("mag: %f", gfc_vector3d_magnitude(cpos));
	gfc_vector3d_set_magnitude(&cpos, edata->cameraDistance);
	//gfc_vector2d_add(cpos, gfc_vector2d_from_angle(edata->cameraYaw), self->position);
	if (edata->selectedBlock) {
		gfc_vector3d_copy(edata->selectedBlock->position, self->position);
		gfc_vector3d_copy(edata->selectedBlock->rotation, self->rotation);
	}
	gfc_vector3d_add(cpos, cpos, self->position);
	gf3d_camera_look_at(self->position, &cpos);

	if (edata->selectedNode) {
		node_free(edata->selectedNode);
	}
	if (edata->nodeMode) {
		edata->selectedNode = spawn_node(self->position);
		if (!edata->selectedNode) {
			edata->nodeMode = 0;
			edata->selectedBlock = spawn_block(edata->currcycle);
			edata->selectedBlock->colormod = GFC_COLOR_LIGHTGREEN;
		}

	}
	

}

void editor_free(Entity* self) {
	if (!self) {
		slog("invalid self pointer for editor_free");
		return;
	}
	editorData* edata = self->data;
	if (!edata) {
		slog("no editor data to free");
		return;
	}

	free(edata);
	memset(self, 0, sizeof(Entity));
}

void editor_draw(Entity* self) {
	if (!self) return;
	editorData* edata = self->data;
	if (!edata) return;
	if (!edata->selectedBlock) return;
	gf3d_draw_edge_3d(gfc_edge3d(0, 0, 0, 0, 6, 0), self->position, edata->selectedBlock->rotation, gfc_vector3d(1, 1, 1), 0.5, GFC_COLOR_BLUE);
	//gf3d_draw_edge_3d(gfc_edge3d(2, 4, 0, 0, 6, 0), self->position, edata->selectedBlock->rotation, gfc_vector3d(1, 1, 1), 0.5, GFC_COLOR_BLUE);
	//gf3d_draw_edge_3d(gfc_edge3d(-2, 4, 0, 0, 6, 0), self->position, edata->selectedBlock->rotation, gfc_vector3d(1, 1, 1), 0.5, GFC_COLOR_BLUE);
}

Entity* editor_spawn(mapData* mdata) {
	Entity* editor = entity_new();
	editorData* edata;
	if (!editor) {
		slog("Could not create editor entity");
		return 0;
	}
	edata = gfc_allocate_array(sizeof(editorData), 1);
	if (!edata) {
		slog("Could not allocate editor data");
		return 0;
	}
	memset(edata, 0, sizeof(editorData));
	editor->free = editor_free;
	editor->update = editor_update;
	editor->think = editor_think;
	editor->data = edata;
	editor->position = gfc_vector3d(0, 0, 10);
	editor->draw = editor_draw;

	edata->cameraPitch = -GFC_HALF_PI*0.5;
	edata->cameraYaw = 0;
	edata->cameraDistance = 50;

	edata->mdata = mdata;
	
	//slog("spawnenennd");
	return editor;
}

