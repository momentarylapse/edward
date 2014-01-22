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

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();

	virtual void OnDrawWin(MultiView::Window *win);

	void UpdateGeometry();

private:
	vector pos, axis;
	float radius1, radius2;
	bool pos_chosen, rad_chosen;
	Geometry *geo;
};

#endif /* MODEMODELMESHCREATETORUS_H_ */
