/*
 * ModeModelMeshDeformCylinder.h
 *
 *  Created on: 26.12.2014
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORMCYLINDER_H_
#define SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORMCYLINDER_H_

#include "../../../ModeCreation.h"
#include "../../../../data/model/DataModel.h"
#include "../../../../lib/math/interpolation.h"
#include "../../../../lib/math/vec3.h"

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

	void on_draw_post() override;
	void on_draw_win(MultiView::Window *win) override;

	void on_ok();

	void update_hover();
	void update_params();

//private:
	Geometry *geo;

	vec3 transform(const vec3 &v);
	void preview();
	void restore();

	vec3 dir;
	vec3 axis[2];
	float radius;
	Array<vec3> param;
	int hover;
	vec3 hover_tp;
	vec3 orig_param;

	Interpolator<float> *inter;

	bool has_preview;
	Array<int> index;
	Array<vec3> old_pos;
};

#endif /* SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORMCYLINDER_H_ */
