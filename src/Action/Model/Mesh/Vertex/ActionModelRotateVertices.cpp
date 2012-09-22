/*
 * ActionModelRotateVertices.cpp
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#include "ActionModelRotateVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

ActionModelRotateVertices::ActionModelRotateVertices(Data *d, const vector & _pos0) :
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

ActionModelRotateVertices::~ActionModelRotateVertices()
{
}



void ActionModelRotateVertices::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Vertex[i].pos = old_data[ii];
	m->SetNormalsDirtyByVertices(index);
}



void *ActionModelRotateVertices::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	matrix rot;
	MatrixRotation(rot, param);
	foreachi(int i, index, ii)
		m->Vertex[i].pos = pos0 + rot * (old_data[ii] - pos0);
	m->SetNormalsDirtyByVertices(index);
	return NULL;
}


