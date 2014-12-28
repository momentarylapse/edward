/*
 * ModeModelMeshCreateCylinder.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATECYLINDER_H_
#define MODEMODELMESHCREATECYLINDER_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class Geometry;

class ModeModelMeshCreateCylinder: public ModeCreation<DataModel>, public HuiEventHandler
{
public:
	ModeModelMeshCreateCylinder(ModeBase *parent);
	virtual ~ModeModelMeshCreateCylinder();

	virtual void onStart();
	virtual void onEnd();

	virtual void onMouseMove();
	virtual void onLeftButtonUp();

	virtual void onDrawWin(MultiView::Window *win);

	void updateGeometry();

	void onClose();

private:
	Array<vector> pos;
	vector length;
	float radius;
	Geometry *geo;
};

#endif /* MODEMODELMESHCREATECYLINDER_H_ */
