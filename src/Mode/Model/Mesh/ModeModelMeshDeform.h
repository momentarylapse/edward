/*
 * ModeModelMeshDeform.h
 *
 *  Created on: 16.01.2020
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_MODEMODELMESHDEFORM_H_
#define SRC_MODE_MODEL_MESH_MODEMODELMESHDEFORM_H_

#include "ModeModelMesh.h"

class BrushPanel;

class ModeModelMeshDeform: public Mode<DataModel>, public Observable {
public:
	ModeModelMeshDeform(ModeBase *parent);
	virtual ~ModeModelMeshDeform();
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

	void on_diameter_slider();
	void on_depth_slider();

	Action *get_action();
	void apply();

private:
	bool brushing;
	float distance;
	vector last_pos;
};

extern ModeModelMeshDeform *mode_model_mesh_deform;

#endif /* SRC_MODE_MODEL_MESH_MODEMODELMESHDEFORM_H_ */
