/*
 * ModeModelMeshCreateCylinderSnake.h
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATECYLINDERSNAKE_H_
#define MODEMODELMESHCREATECYLINDERSNAKE_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class Geometry;

class ModeModelMeshCreateCylinderSnake: public ModeCreation<DataModel> {
public:
	ModeModelMeshCreateCylinderSnake(ModeBase *parent);

	void on_start() override;
	void on_end() override;

	void on_mouse_move() override;
	void on_left_button_up() override;
	void on_command(const string& id) override;

	void on_draw_win(MultiView::Window *win) override;

	void update_geometry();

private:
	Array<vector> pos;
	bool ready_for_scaling;
	vector length;
	float radius;
	bool closed;
	Geometry *geo;
};

#endif /* MODEMODELMESHCREATECYLINDERSNAKE_H_ */
