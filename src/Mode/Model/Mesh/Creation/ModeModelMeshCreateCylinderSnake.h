/*
 * ModeModelMeshCreateCylinderSnake.h
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATECYLINDERSNAKE_H_
#define MODEMODELMESHCREATECYLINDERSNAKE_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModelGeometry;

class ModeModelMeshCreateCylinderSnake: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreateCylinderSnake(ModeBase *parent);
	virtual ~ModeModelMeshCreateCylinderSnake();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();
	virtual void OnKeyDown();

	virtual void OnDrawWin(MultiViewWindow *win);

	void UpdateGeometry();

private:
	Array<vector> pos;
	bool ready_for_scaling;
	vector length;
	float radius;
	bool closed;
	ModelGeometry *geo;
};

#endif /* MODEMODELMESHCREATECYLINDERSNAKE_H_ */
