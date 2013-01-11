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

class ModeModelMeshCreateCylinderSnake: public ModeCreation
{
public:
	ModeModelMeshCreateCylinderSnake(Mode *_parent);
	virtual ~ModeModelMeshCreateCylinderSnake();

	void OnStart();
	void OnEnd();

	void OnMouseMove();
	void OnLeftButtonDown();
	void OnKeyDown();

	void OnDrawWin(int win);

	void UpdateGeometry();

	DataModel *data;
	Data *GetData(){	return data;	}

private:
	Array<vector> pos;
	bool ready_for_scaling;
	vector length;
	float radius;
	ModelGeometry *geo;
};

#endif /* MODEMODELMESHCREATECYLINDERSNAKE_H_ */
