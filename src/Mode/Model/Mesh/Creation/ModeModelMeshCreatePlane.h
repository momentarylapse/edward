/*
 * ModeModelMeshCreatePlane.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATEPLANE_H_
#define MODEMODELMESHCREATEPLANE_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshCreatePlane: public ModeCreation
{
public:
	ModeModelMeshCreatePlane(Mode *_parent);
	virtual ~ModeModelMeshCreatePlane();

	void OnStart();
	void OnEnd();

	void OnMouseMove();
	void OnLeftButtonDown();

	void OnDrawWin(MultiViewWindow *win);

	DataModel *data;
	Data *GetData(){	return data;	}

private:
	vector pos, pos2;
	vector length[2];
	bool pos_chosen;
	bool invert;
};

#endif /* MODEMODELMESHCREATEPLANE_H_ */
