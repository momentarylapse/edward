/*
 * ModeModelMeshPaint.h
 *
 *  Created on: 16.01.2020
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_MODEMODELMESHPAINT_H_
#define SRC_MODE_MODEL_MESH_MODEMODELMESHPAINT_H_

#include "ModeModelMesh.h"

class BrushPanel;

class ModeModelMeshPaint: public Mode<ModeModelMesh, DataModel> {
public:
	ModeModelMeshPaint(ModeModelMesh *parent, MultiView::MultiView *mv);
	void on_start() override;
	void on_end() override;

	BrushPanel *dialog;

	void on_selection_change() override;
	void on_set_multi_view() override;
	void on_draw() override;
	void on_draw_win(MultiView::Window *win) override;


	void on_left_button_down() override;
	void on_left_button_up() override;
	void on_mouse_move() override;

	Action *get_action();
	void apply();
	float radius();

private:
	bool brushing;
	float distance;
	vec3 last_pos;
};

extern ModeModelMeshPaint *mode_model_mesh_paint;

#endif /* SRC_MODE_MODEL_MESH_MODEMODELMESHPAINT_H_ */
