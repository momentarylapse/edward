/*
 * ModeModelMeshCreateTorus.h
 *
 *  Created on: 29.09.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATETORUS_H_
#define MODEMODELMESHCREATETORUS_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshCreateTorus: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreateTorus(ModeBase *parent);
	virtual ~ModeModelMeshCreateTorus();

	virtual void onStart();
	virtual void onEnd();

	virtual void onMouseMove();
	virtual void onLeftButtonUp();

	virtual void onDrawWin(MultiView::Window *win);

	void onClose();

	void updateGeometry();

private:
	vector pos, axis;
	float radius1, radius2;
	bool pos_chosen, rad_chosen;
	Geometry *geo;
};

#endif /* MODEMODELMESHCREATETORUS_H_ */
