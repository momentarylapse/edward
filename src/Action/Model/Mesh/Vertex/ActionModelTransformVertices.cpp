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
	foreachi(ModelVertex &v, d->vertex, i)
		if (v.is_selected){
			index.add(i);
			old_data.add(d->vertex[i].pos);
		}
}

ActionModelTransformVertices::~ActionModelTransformVertices()
{
}



void ActionModelTransformVertices::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->vertex[i].pos = old_data[ii];
	m->SetNormalsDirtyByVertices(index);
}



void *ActionModelTransformVertices::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreach(int i, index)
		m->vertex[i].pos =  mat * m->vertex[i].pos;
	m->SetNormalsDirtyByVertices(index);
	return NULL;
}


