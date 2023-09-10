/*
 * ModeModelMeshExtrudePolygons.h
 *
 *  Created on: 28.09.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHEXTRUDEPOLYGONS_H_
#define MODEMODELMESHEXTRUDEPOLYGONS_H_

#include "../ModeModelMesh.h"
#include "../../../ModeCreation.h"
#include "../../../../data/model/DataModel.h"
#include "../../../../data/model/ModelSelection.h"

class ModeModelMeshExtrudePolygons: public ModeCreation<ModeModelMesh, DataModel> {
public:
	ModeModelMeshExtrudePolygons(ModeModelMesh *parent, bool independent);

	void on_start() override;
	void on_end() override;

	void on_left_button_down() override;
	void on_mouse_move() override;

	void on_draw_win(MultiView::Window *win) override;

	void preview();
	void cleanUp();

private:
	float offset;
	bool independent;
	ModelSelection selection;
};

#endif /* MODEMODELMESHEXTRUDEPOLYGONS_H_ */
