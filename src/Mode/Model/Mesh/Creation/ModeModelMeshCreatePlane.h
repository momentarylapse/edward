/*
 * ModeModelMeshCreatePlane.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATEPLANE_H_
#define MODEMODELMESHCREATEPLANE_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshCreatePlane: public ModeCreation<DataModel> {
public:
	ModeModelMeshCreatePlane(ModeBase *parent);

	void on_start() override;
	void on_end() override;

	void on_mouse_move() override;
	void on_left_button_up() override;

	void on_draw_win(MultiView::Window *win) override;

private:
	vector pos, pos2;
	vector length[2];
	bool pos_chosen;
	bool invert;
};

#endif /* MODEMODELMESHCREATEPLANE_H_ */
