/*
 * ModeModelMeshDeformCylinder.h
 *
 *  Created on: 26.12.2014
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORMCYLINDER_H_
#define SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORMCYLINDER_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class Geometry;

class ModeModelMeshDeformCylinder: public ModeCreation<DataModel> {
public:
	ModeModelMeshDeformCylinder(ModeBase *parent);
	virtual ~ModeModelMeshDeformCylinder();

	void on_start() override;
	void on_end() override;

	void on_mouse_move() override;
	void on_left_button_down() override;
	void on_key_down(int key_code) override;
	void on_command(const string &id) override;

	void on_draw_win(MultiView::Window *win) override;

	void on_ok();

	void update_hover();
	void update_params();

//private:
	Geometry *geo;

	vector transform(const vector &v);
	void preview();
	void restore();

	vector dir;
	vector axis[2];
	float radius;
	Array<vector> param;
	int hover;
	vector hover_tp;
	vector orig_param;

	Interpolator<float> *inter;

	bool has_preview;
	Array<int> index;
	Array<vector> old_pos;
};

#endif /* SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORMCYLINDER_H_ */
