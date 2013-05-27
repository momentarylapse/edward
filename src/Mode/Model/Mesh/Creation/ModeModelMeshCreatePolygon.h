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

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnLeftButtonDown();
	virtual void OnKeyDown();

	virtual void OnDrawWin(MultiViewWindow *win);

	Array<int> selection;
};

#endif /* MODEMODELMESHCREATEPOLYGON_H_ */
