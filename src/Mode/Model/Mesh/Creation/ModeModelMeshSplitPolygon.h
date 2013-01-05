/*
 * ModeModelMeshSplitPolygon.h
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHSPLITPOLYGON_H_
#define MODEMODELMESHSPLITPOLYGON_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshSplitPolygon: public ModeCreation
{
public:
	ModeModelMeshSplitPolygon(Mode *_parent);
	virtual ~ModeModelMeshSplitPolygon();

	void OnLeftButtonDown();

	void OnDrawWin(int win);

	DataModel *data;
	Data *GetData(){	return data;	}

private:
	int surface, triangle;
	vector pos;
	int edge;
	float factor;
};

#endif /* MODEMODELMESHSPLITPOLYGON_H_ */
