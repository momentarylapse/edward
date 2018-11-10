/*
 * ModeModelMeshSplitPolygon.h
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHSPLITPOLYGON_H_
#define MODEMODELMESHSPLITPOLYGON_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshSplitPolygon: public ModeCreation<DataModel>
{
public:
	ModeModelMeshSplitPolygon(ModeBase *parent);

	void on_start() override;

	void on_left_button_up() override;

	void on_draw_win(MultiView::Window *win) override;

private:
	int surface, triangle;
	vector pos;
	int edge;
	float factor;
};

#endif /* MODEMODELMESHSPLITPOLYGON_H_ */
