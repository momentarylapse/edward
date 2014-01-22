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
	virtual ~ModeModelMeshExtrudePolygons();

	virtual void OnEnd();

	virtual void OnLeftButtonDown();
	virtual void OnMouseMove();

	virtual void OnDrawWin(MultiView::Window *win);

	void Preview();
	void CleanUp();

private:
	float offset;
	ModelSelectionState selection;
};

#endif /* MODEMODELMESHEXTRUDEPOLYGONS_H_ */
