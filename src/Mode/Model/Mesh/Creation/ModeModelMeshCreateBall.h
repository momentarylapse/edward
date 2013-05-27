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

class ModelGeometry;

class ModeModelMeshCreateBall: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreateBall(ModeBase *_parent);
	virtual ~ModeModelMeshCreateBall();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();

	virtual void OnDrawWin(MultiViewWindow *win);

	void UpdateGeometry();

private:
	vector pos;
	float radius;
	bool pos_chosen;
	ModelGeometry *geo;
};

#endif /* MODEMODELMESHCREATEBALL_H_ */
