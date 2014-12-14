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

class ModeModelMeshCreatePlane: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreatePlane(ModeBase *parent);
	virtual ~ModeModelMeshCreatePlane();

	virtual void onStart();
	virtual void onEnd();

	virtual void onMouseMove();
	virtual void onLeftButtonDown();

	virtual void onDrawWin(MultiView::Window *win);

private:
	vector pos, pos2;
	vector length[2];
	bool pos_chosen;
	bool invert;
};

#endif /* MODEMODELMESHCREATEPLANE_H_ */
