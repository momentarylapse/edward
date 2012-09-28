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

class ModeModelMeshCreateTorus: public ModeCreation
{
public:
	ModeModelMeshCreateTorus(Mode *_parent);
	virtual ~ModeModelMeshCreateTorus();

	void OnStart();
	void OnEnd();

	void OnMouseMove();
	void OnLeftButtonDown();

	void OnDrawWin(int win, irect dest);

	DataModel *data;
	Data *GetData(){	return data;	}

private:
	vector pos, axis;
	float radius1, radius2;
	bool pos_chosen, rad_chosen;
};

#endif /* MODEMODELMESHCREATETORUS_H_ */
