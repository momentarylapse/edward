/*
 * ModeModelMeshCreatePlatonic.h
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATEPLATONIC_H_
#define MODEMODELMESHCREATEPLATONIC_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"
class Geometry;

class ModeModelMeshCreatePlatonic: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreatePlatonic(ModeBase *parent, int type);
	virtual ~ModeModelMeshCreatePlatonic();

	virtual void onStart();
	virtual void onEnd();

	virtual void onMouseMove();
	virtual void onLeftButtonDown();

	virtual void onDrawWin(MultiView::Window *win);

	void UpdateGeometry();

private:
	Geometry *geo;
	int type;
	vector pos;
	float radius;
	bool pos_chosen;
};

#endif /* MODEMODELMESHCREATEPLATONIC_H_ */
