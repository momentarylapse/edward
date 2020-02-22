/*
 * ModeModelMeshDeform.h
 *
 *  Created on: 16.01.2020
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_MODEMODELMESHDEFORM_H_
#define SRC_MODE_MODEL_MESH_MODEMODELMESHDEFORM_H_

#include "ModeModelMesh.h"

class DeformBrushPanel;

class ModeModelMeshDeform: public Mode<DataModel> {
public:
	ModeModelMeshDeform(ModeBase *parent);
	virtual ~ModeModelMeshDeform();
	void on_start() override;
	void on_end() override;

	DeformBrushPanel *dialog;

	void on_selection_change() override;
	void on_set_multi_view() override;
	void on_draw() override;
	void on_draw_win(MultiView::Window *win) override;


	void on_left_button_down() override;
	void on_left_button_up() override;
	void on_mouse_move() override;

	Action *get_action();
	void apply();

private:
	bool brushing;
	float distance;
	vector last_pos;
};

extern ModeModelMeshDeform *mode_model_mesh_deform;

#endif /* SRC_MODE_MODEL_MESH_MODEMODELMESHDEFORM_H_ */
