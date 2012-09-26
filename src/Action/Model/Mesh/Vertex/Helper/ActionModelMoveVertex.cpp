/*
 * ActionModelMoveVertex.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ActionModelMoveVertex.h"
#include "../../../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelMoveVertex::ActionModelMoveVertex(int _vertex, const vector &_pos)
{
	vertex = _vertex;
	pos = _pos;
}

void *ActionModelMoveVertex::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	vector temp = m->Vertex[vertex].pos;
	m->Vertex[vertex].pos = pos;
	pos = temp;
	return NULL;
}

void ActionModelMoveVertex::undo(Data *d)
{
	execute(d);
}
