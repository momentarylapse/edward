/*
 * ModeModelMeshCreateBall.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATEBALL_H_
#define MODEMODELMESHCREATEBALL_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class Geometry;

class ModeModelMeshCreateBall: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreateBall(ModeBase *_parent);
	virtual ~ModeModelMeshCreateBall();

	virtual void onStart();
	virtual void onEnd();

	virtual void onMouseMove();
	virtual void onLeftButtonUp();

	virtual void onDrawWin(MultiView::Window *win);

	void updateGeometry();
	void onTypeBall();
	void onTypeSphere();

private:
	vector pos;
	float radius;
	bool pos_chosen;
	Geometry *geo;
};

#endif /* MODEMODELMESHCREATEBALL_H_ */
