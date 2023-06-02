/*
 * ModeModelMeshSplitPolygon.h
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHSPLITPOLYGON_H_
#define MODEMODELMESHSPLITPOLYGON_H_

#include "../../../ModeCreation.h"
#include "../../../../data/model/DataModel.h"

class ModeModelMeshSplitPolygon: public ModeCreation<DataModel> {
public:
	ModeModelMeshSplitPolygon(ModeBase *parent);

	void on_start() override;

	void on_left_button_up() override;

	void on_draw_win(MultiView::Window *win) override;

private:
	vec3 pos = v_0;
	int polygon = -1, edge = -1;
	float factor = 0;
};

#endif /* MODEMODELMESHSPLITPOLYGON_H_ */
