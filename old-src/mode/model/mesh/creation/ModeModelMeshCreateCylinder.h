/*
 * ModeModelMeshCreateCylinder.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATECYLINDER_H_
#define MODEMODELMESHCREATECYLINDER_H_

#include "../../../ModeCreation.h"
#include "../../../../data/model/DataModel.h"

class Geometry;
class ModeModelMesh;

class ModeModelMeshCreateCylinder: public ModeCreation<ModeModelMesh, DataModel> {
public:
	ModeModelMeshCreateCylinder(ModeModelMesh *parent);
	virtual ~ModeModelMeshCreateCylinder();

	void on_start() override;
	void on_end() override;

	void on_mouse_move() override;
	void on_left_button_up() override;

	void on_draw_win(MultiView::Window *win) override;

	void update_geometry();

private:
	Array<vec3> pos;
	vec3 length;
	float radius;
	Geometry *geo;
};

#endif /* MODEMODELMESHCREATECYLINDER_H_ */
