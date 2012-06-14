/*
 * ModeModelMeshCreateTriangles.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATETRIANGLES_H_
#define MODEMODELMESHCREATETRIANGLES_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshCreateTriangles: public ModeCreation
{
public:
	ModeModelMeshCreateTriangles(Mode *_parent);
	virtual ~ModeModelMeshCreateTriangles();

	void OnStart();
	void OnEnd();

	void OnLeftButtonDown();
	void OnKeyDown();

	void OnDrawWin(int win, irect dest);

	DataModel *data;
	Data *GetData(){	return data;	}
	Array<int> selection;
};

#endif /* MODEMODELMESHCREATETRIANGLES_H_ */
