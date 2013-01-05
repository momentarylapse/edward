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

class ModeModelMeshCreateCylinder: public ModeCreation
{
public:
	ModeModelMeshCreateCylinder(Mode *_parent);
	virtual ~ModeModelMeshCreateCylinder();

	void OnStart();
	void OnEnd();

	void OnMouseMove();
	void OnLeftButtonDown();
	void OnKeyDown();

	void OnDrawWin(int win);

	DataModel *data;
	Data *GetData(){	return data;	}

private:
	Array<vector> pos;
	bool ready_for_scaling;
	vector length;
	float radius;
};

#endif /* MODEMODELMESHCREATECYLINDER_H_ */
