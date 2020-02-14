/*
 * ActionModelTransformVertices.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "ActionModelTransformVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/ModelMesh.h"

ActionModelTransformVertices::ActionModelTransformVertices(DataModel *d) :
	ActionMultiView()
{
	// list of selected vertices and save old pos
	foreachi(ModelVertex &v, d->edit_mesh->vertex, i)
		if (v.is_selected){
			index.add(i);
			old_data.add(d->edit_mesh->vertex[i].pos);
		}
}

ActionModelTransformVertices::~ActionModelTransformVertices()
{
}



void ActionModelTransformVertices::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->edit_mesh->vertex[i].pos = old_data[ii];
	m->setNormalsDirtyByVertices(index);
}



void *ActionModelTransformVertices::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	for (int i: index)
		m->edit_mesh->vertex[i].pos =  mat * m->edit_mesh->vertex[i].pos;
	m->setNormalsDirtyByVertices(index);
	return NULL;
}


