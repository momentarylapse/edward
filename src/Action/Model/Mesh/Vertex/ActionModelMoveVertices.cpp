/*
 * ActionModelMoveVertices.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "ActionModelMoveVertices.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelMoveVertices::ActionModelMoveVertices(DataModel *d, const vector &_param, const vector &_pos0) :
	ActionMultiView(_param, _pos0)
{
	// list of selected vertices and save old pos
	foreachi(ModelVertex &v, d->Vertex, i)
		if (v.is_selected){
			index.add(i);
			old_data.add(d->Vertex[i].pos);
		}
}

ActionModelMoveVertices::~ActionModelMoveVertices()
{
}



void ActionModelMoveVertices::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Vertex[i].pos = old_data[ii];
	m->SetNormalsDirtyByVertices(index);
}



void *ActionModelMoveVertices::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreach(int i, index)
		m->Vertex[i].pos +=  param;
	m->SetNormalsDirtyByVertices(index);
	return NULL;
}


