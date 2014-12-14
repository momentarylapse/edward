/*
 * ModeModelMeshCreatePolygon.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATEPOLYGON_H_
#define MODEMODELMESHCREATEPOLYGON_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshCreatePolygon: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreatePolygon(ModeBase *parent);
	virtual ~ModeModelMeshCreatePolygon();

	virtual void onStart();
	virtual void onEnd();

	virtual void onLeftButtonDown();
	virtual void onKeyDown();

	virtual void onDrawWin(MultiView::Window *win);

	Array<int> selection;
};

#endif /* MODEMODELMESHCREATEPOLYGON_H_ */
