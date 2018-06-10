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

	void onStart() override;
	void onEnd() override;

	void onLeftButtonDown() override;
	void onMouseMove() override;

	void onDrawWin(MultiView::Window *win) override;

	void preview();
	void cleanUp();

private:
	float offset;
	ModelSelectionState selection;
};

#endif /* MODEMODELMESHEXTRUDEPOLYGONS_H_ */
