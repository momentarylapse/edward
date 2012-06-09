/*
 * ActionModelMVMoveVertices.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "ActionModelMVMoveVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

ActionModelMVMoveVertices::ActionModelMVMoveVertices(Data *d, const vector &_pos0) :
	ActionMultiView(d, _pos0)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// list of selected vertices and save old pos
	foreachi(m->Vertex, v, i)
		if (v.is_selected){
			index.add(i);
			old_data.add(m->Vertex[i].pos);
		}
}

ActionModelMVMoveVertices::~ActionModelMVMoveVertices()
{
}

void ActionModelMVMoveVertices::abort(Data *d)
{
	msg_write("move vert abort");
	undo(d);
}



void ActionModelMVMoveVertices::undo(Data *d)
{
	msg_write("move vert undo");
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii)
		m->Vertex[i].pos = old_data[ii];
	m->SetNormalsDirtyByVertices(index);
}



void *ActionModelMVMoveVertices::execute(Data *d)
{
	msg_write("move vert do");
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii)
		m->Vertex[i].pos = old_data[ii] + param;
	m->SetNormalsDirtyByVertices(index);
	return NULL;
}


