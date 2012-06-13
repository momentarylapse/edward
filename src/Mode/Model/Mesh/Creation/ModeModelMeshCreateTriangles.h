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
	ModeModelMeshCreateTriangles(Mode *_parent, DataModel *_data);
	virtual ~ModeModelMeshCreateTriangles();

	void Start();
	void End();

	void OnLeftButtonDown();
	void OnKeyDown();

	void PostDrawWin(int win, irect dest);

	DataModel *data;
	Data *GetData(){	return data;	}
	Array<int> selection;
};

#endif /* MODEMODELMESHCREATETRIANGLES_H_ */
