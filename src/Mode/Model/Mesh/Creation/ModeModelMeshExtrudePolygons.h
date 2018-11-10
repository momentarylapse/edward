/*
 * ModeModelMeshExtrudePolygons.h
 *
 *  Created on: 28.09.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHEXTRUDEPOLYGONS_H_
#define MODEMODELMESHEXTRUDEPOLYGONS_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshExtrudePolygons: public ModeCreation<DataModel>
{
public:
	ModeModelMeshExtrudePolygons(ModeBase *parent);

	void on_start() override;
	void on_end() override;

	void on_left_button_down() override;
	void on_mouse_move() override;

	void on_draw_win(MultiView::Window *win) override;

	void preview();
	void cleanUp();

private:
	float offset;
	ModelSelectionState selection;
};

#endif /* MODEMODELMESHEXTRUDEPOLYGONS_H_ */
