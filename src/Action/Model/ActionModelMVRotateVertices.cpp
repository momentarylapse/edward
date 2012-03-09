/*
 * ActionModelMVRotateVertices.cpp
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#include "ActionModelMVRotateVertices.h"
#include "../../Data/Model/DataModel.h"
#include "../../lib/file/file.h"
#include "../../lib/types/types.h"

ActionModelMVRotateVertices::ActionModelMVRotateVertices(Data *d, int _set_no, const Array<int> & _index, const vector & _pos0) :
	ActionMultiView(d, _set_no, _index, _pos0)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// save old pos
	old_data.resize(_index.num);
	foreachi(index, i, ii)
		old_data[ii] = m->Vertex[i].pos;
}

ActionModelMVRotateVertices::~ActionModelMVRotateVertices()
{
}



void ActionModelMVRotateVertices::undo(Data *d)
{
	msg_write("rotate vert undo");
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii)
		m->Vertex[i].pos = old_data[ii];
	m->SetNormalsDirtyByVertices(index);
}



void ActionModelMVRotateVertices::redo(Data *d)
{
	execute(d);
}



void *ActionModelMVRotateVertices::execute(Data *d)
{
	msg_write("rotate vert do");
	DataModel *m = dynamic_cast<DataModel*>(d);
	matrix rot;
	MatrixRotation(rot, param);
	foreachi(index, i, ii)
		m->Vertex[i].pos = pos0 + (old_data[ii] - pos0) * rot;
	m->SetNormalsDirtyByVertices(index);
	return NULL;
}

void ActionModelMVRotateVertices::set_param(Data *d, const vector & _param)
{
	//msg_write("rotate vert set param");
	param = _param;
	execute(d);
}



void ActionModelMVRotateVertices::abort(Data *d)
{
	msg_write("rotate vert abort");
	undo(d);
}


