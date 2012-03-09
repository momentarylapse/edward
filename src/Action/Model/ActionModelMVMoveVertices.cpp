/*
 * ActionModelMVMoveVertices.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "ActionModelMVMoveVertices.h"
#include "../../Data/Model/DataModel.h"
#include "../../lib/file/file.h"
#include "../../lib/types/types.h"

ActionModelMVMoveVertices::ActionModelMVMoveVertices(Data *d, int _set_no, const Array<int> &_index, const vector &_pos0) :
	ActionMultiView(d, _set_no, _index, _pos0)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// save old pos
	old_data.resize(_index.num);
	foreachi(index, i, ii)
		old_data[ii] = m->Vertex[i].pos;
}

ActionModelMVMoveVertices::~ActionModelMVMoveVertices()
{
}

void ActionModelMVMoveVertices::abort(Data *d)
{
	msg_write("move vert abort");
	undo(d);
}



void ActionModelMVMoveVertices::set_param(Data *d, const vector &_param)
{
	//msg_write("move vert set param");
	param = _param;
	execute(d);
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



void ActionModelMVMoveVertices::redo(Data *d)
{
	execute(d);
}


