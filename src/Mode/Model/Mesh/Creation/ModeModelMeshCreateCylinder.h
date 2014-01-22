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

class ModeModelMeshCreateCylinder: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreateCylinder(ModeBase *parent);
	virtual ~ModeModelMeshCreateCylinder();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();

	virtual void OnDrawWin(MultiView::Window *win);

	void UpdateGeometry();

private:
	Array<vector> pos;
	vector length;
	float radius;
	Geometry *geo;
};

#endif /* MODEMODELMESHCREATECYLINDER_H_ */
