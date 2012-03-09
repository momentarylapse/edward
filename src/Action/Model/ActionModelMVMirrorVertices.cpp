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

ActionModelMVMirrorVertices::ActionModelMVMirrorVertices(Data *d, int _set_no, const Array<int> &_index, const vector &_pos0) :
	ActionMultiView(d, _set_no, _index, _pos0)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// save old pos
	old_data.resize(_index.num);
	foreachi(index, i, ii)
		old_data[ii] = m->Vertex[i].pos;
}

ActionModelMVMirrorVertices::~ActionModelMVMirrorVertices()
{
}



void ActionModelMVMirrorVertices::redo(Data *d)
{
	execute(d);
}



void ActionModelMVMirrorVertices::set_param(Data *d, const vector & _param)
{
	//msg_write("mirror vert set param");
	param = _param;
	execute(d);
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


