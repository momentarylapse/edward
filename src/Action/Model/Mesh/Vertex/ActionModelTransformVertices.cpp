/*
 * ActionModelTransformVertices.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "ActionModelTransformVertices.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelTransformVertices::ActionModelTransformVertices(DataModel *d) :
	ActionMultiView()
{
	// list of selected vertices and save old pos
	foreachi(ModelVertex &v, d->Vertex, i)
		if (v.is_selected){
			index.add(i);
			old_data.add(d->Vertex[i].pos);
		}
}

ActionModelTransformVertices::~ActionModelTransformVertices()
{
}



void ActionModelTransformVertices::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Vertex[i].pos = old_data[ii];
	m->SetNormalsDirtyByVertices(index);
}



void *ActionModelTransformVertices::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreach(int i, index)
		m->Vertex[i].pos =  mat * m->Vertex[i].pos;
	m->SetNormalsDirtyByVertices(index);
	return NULL;
}


