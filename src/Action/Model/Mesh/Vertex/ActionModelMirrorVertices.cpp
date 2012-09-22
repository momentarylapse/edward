/*
 * ActionModelMirrorVertices.cpp
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#include "ActionModelMirrorVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

ActionModelMirrorVertices::ActionModelMirrorVertices(Data *d, const vector &_pos0) :
	ActionMultiView(d, _pos0)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// list of selected vertices and save old pos
	foreachi(ModelVertex &v, m->Vertex, i)
		if (v.is_selected){
			index.add(i);
			old_data.add(m->Vertex[i].pos);
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


