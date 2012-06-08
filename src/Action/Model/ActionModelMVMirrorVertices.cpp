/*
 * ActionModelMVMirrorVertices.cpp
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#include "ActionModelMVMirrorVertices.h"
#include "../../Data/Model/DataModel.h"
#include "../../lib/file/file.h"
#include "../../lib/types/types.h"

ActionModelMVMirrorVertices::ActionModelMVMirrorVertices(Data *d, const vector &_pos0) :
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

ActionModelMVMirrorVertices::~ActionModelMVMirrorVertices()
{
}



void *ActionModelMVMirrorVertices::execute(Data *d)
{
	msg_write("mirror vert do");
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii)
		m->Vertex[i].pos = old_data[ii] - 2 * param * ((old_data[ii] - pos0) * param);
	m->SetNormalsDirtyByVertices(index);
	return NULL;
}



void ActionModelMVMirrorVertices::undo(Data *d)
{
	msg_write("mirror vert undo");
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii)
		m->Vertex[i].pos = old_data[ii];
	m->SetNormalsDirtyByVertices(index);
}



void ActionModelMVMirrorVertices::abort(Data *d)
{
	msg_write("mirror vert abort");
	undo(d);
}


