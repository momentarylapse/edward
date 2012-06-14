/*
 * ModeModelMeshSplitTriangle.h
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHSPLITTRIANGLE_H_
#define MODEMODELMESHSPLITTRIANGLE_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshSplitTriangle: public ModeCreation
{
public:
	ModeModelMeshSplitTriangle(Mode *_parent);
	virtual ~ModeModelMeshSplitTriangle();

	void OnLeftButtonDown();

	void OnDrawWin(int win, irect dest);

	DataModel *data;
	Data *GetData(){	return data;	}

private:
	int surface, triangle;
	vector pos;
	int edge;
};

#endif /* MODEMODELMESHSPLITTRIANGLE_H_ */
