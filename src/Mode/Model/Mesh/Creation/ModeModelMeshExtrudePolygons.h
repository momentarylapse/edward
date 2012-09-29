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

class ModeModelMeshExtrudePolygons: public ModeCreation
{
public:
	ModeModelMeshExtrudePolygons(Mode *_parent);
	virtual ~ModeModelMeshExtrudePolygons();

	void OnEnd();

	void OnLeftButtonDown();
	void OnMouseMove();

	void OnDrawWin(int win, irect dest);

	void Preview();
	void CleanUp();

	DataModel *data;
	Data *GetData(){	return data;	}

private:
	float offset;
	ModelSelectionState selection;
};

#endif /* MODEMODELMESHEXTRUDEPOLYGONS_H_ */
