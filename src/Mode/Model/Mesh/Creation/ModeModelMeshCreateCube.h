/*
 * ModeModelMeshCreateCube.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATECUBE_H_
#define MODEMODELMESHCREATECUBE_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class Geometry;

class ModeModelMeshCreateCube: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreateCube(ModeBase *_parent);
	virtual ~ModeModelMeshCreateCube();

	void on_start() override;
	void on_end() override;

	void on_mouse_move() override;
	void on_left_button_up() override;

	void on_draw_win(MultiView::Window *win) override;

	void on_close();

	void update_geometry();

	bool set_dpos3();

private:
	vector pos, pos2;
	vector length[3];
	bool pos_chosen;
	bool pos2_chosen;
	Geometry *geo;
};

#endif /* MODEMODELMESHCREATECUBE_H_ */
