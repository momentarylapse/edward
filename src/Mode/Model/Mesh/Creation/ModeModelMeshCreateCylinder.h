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

class ModelGeometry;

class ModeModelMeshCreateCylinder: public ModeCreation
{
public:
	ModeModelMeshCreateCylinder(Mode *_parent);
	virtual ~ModeModelMeshCreateCylinder();

	void OnStart();
	void OnEnd();

	void OnMouseMove();
	void OnLeftButtonDown();

	void OnDrawWin(MultiViewWindow *win);

	void UpdateGeometry();

	DataModel *data;
	Data *GetData(){	return data;	}

private:
	Array<vector> pos;
	vector length;
	float radius;
	ModelGeometry *geo;
};

#endif /* MODEMODELMESHCREATECYLINDER_H_ */
