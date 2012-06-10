/*
 * ActionModelNearifyVertices.cpp
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#include "ActionModelNearifyVertices.h"
#include <assert.h>

ActionModelNearifyVertices::ActionModelNearifyVertices(DataModel *m)
{
	foreachi(m->Vertex, v, i)
		if (v.is_selected)
			index.add(i);
}

ActionModelNearifyVertices::~ActionModelNearifyVertices()
{
}

void ActionModelNearifyVertices::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index.num == old_pos.num);

	// apply old data
	foreachi(index, vi, i)
		m->Vertex[vi].pos = old_pos[i];

	m->SetNormalsDirtyByVertices(index);
}



void *ActionModelNearifyVertices::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	old_pos.clear();
	vector v = v0;

	foreach(index, i){
		// save old data
		old_pos.add(m->Vertex[i].pos);

		// average
		v += m->Vertex[i].pos;
	}

	if (index.num > 0)
		v /= index.num;

	// apply
	foreach(index, i)
		m->Vertex[i].pos = v;

	m->SetNormalsDirtyByVertices(index);
	return NULL;
}

