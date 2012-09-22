/*
 * ActionModelScaleVertices.cpp
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#include "ActionModelScaleVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

ActionModelScaleVertices::ActionModelScaleVertices(Data *d, const vector &_pos0) :
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

ActionModelScaleVertices::~ActionModelScaleVertices()
{
}

void ActionModelScaleVertices::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Vertex[i].pos = old_data[ii];
	m->SetNormalsDirtyByVertices(index);
}



void *ActionModelScaleVertices::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Vertex[i].pos = pos0 +
			(e[0] * (old_data[ii] - pos0)) * param.x * e[0] +
			(e[1] * (old_data[ii] - pos0)) * param.y * e[1] +
			(e[2] * (old_data[ii] - pos0)) * param.z * e[2];
	m->SetNormalsDirtyByVertices(index);
	return NULL;
}


