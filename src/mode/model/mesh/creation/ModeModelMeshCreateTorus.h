/*
 * ModeModelMeshCreateTorus.h
 *
 *  Created on: 29.09.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATETORUS_H_
#define MODEMODELMESHCREATETORUS_H_

#include "../../../ModeCreation.h"
#include "../../../../data/model/DataModel.h"

class ModeModelMesh;

class ModeModelMeshCreateTorus: public ModeCreation<ModeModelMesh, DataModel> {
public:
	ModeModelMeshCreateTorus(ModeModelMesh *parent);
	virtual ~ModeModelMeshCreateTorus();

	void on_start() override;
	void on_end() override;

	void on_mouse_move() override;
	void on_left_button_up() override;

	void on_draw_win(MultiView::Window *win) override;

	void update_geometry();

private:
	vec3 pos, axis;
	float radius1, radius2;
	bool pos_chosen, rad_chosen;
	Geometry *geo;
};

#endif /* MODEMODELMESHCREATETORUS_H_ */
