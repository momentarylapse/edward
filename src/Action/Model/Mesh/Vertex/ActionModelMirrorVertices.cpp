/*
 * ActionModelMirrorVertices.cpp
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#include "ActionModelMirrorVertices.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelMirrorVertices::ActionModelMirrorVertices(DataModel *d, const vector &_param, const vector &_pos0) :
	ActionMultiView(_param, _pos0)
{
	// list of selected vertices and save old pos
	foreachi(ModelVertex &v, d->Vertex, i)
		if (v.is_selected){
			index.add(i);
			old_data.add(d->Vertex[i].pos);
		}
}

ActionModelMirrorVertices::~ActionModelMirrorVertices()
{
}



void *ActionModelMirrorVertices::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Vertex[i].pos = old_data[ii] - 2 * param * ((old_data[ii] - pos0) * param);
	m->SetNormalsDirtyByVertices(index);
	return NULL;
}



void ActionModelMirrorVertices::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Vertex[i].pos = old_data[ii];
	m->SetNormalsDirtyByVertices(index);
}


