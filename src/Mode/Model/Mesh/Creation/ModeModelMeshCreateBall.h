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

class ModeModelMeshCreateBall: public ModeCreation<DataModel>, public HuiEventHandler
{
public:
	ModeModelMeshCreateBall(ModeBase *_parent);
	virtual ~ModeModelMeshCreateBall();

	virtual void onStart();
	virtual void onEnd();

	virtual void onMouseMove();
	virtual void onLeftButtonDown();

	virtual void onDrawWin(MultiView::Window *win);

	void UpdateGeometry();
	void OnTypeBall();
	void OnTypeSphere();

private:
	vector pos;
	float radius;
	bool pos_chosen;
	Geometry *geo;
};

#endif /* MODEMODELMESHCREATEBALL_H_ */
