/*
 * ActionModelMVScaleVertices.cpp
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#include "ActionModelMVScaleVertices.h"
#include "../../Data/Model/DataModel.h"
#include "../../lib/file/file.h"
#include "../../lib/types/types.h"

ActionModelMVScaleVertices::ActionModelMVScaleVertices(Data *d, const vector &_pos0) :
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

ActionModelMVScaleVertices::~ActionModelMVScaleVertices()
{
}

void ActionModelMVScaleVertices::undo(Data *d)
{
	msg_write("scale vert undo");
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii)
		m->Vertex[i].pos = old_data[ii];
	m->SetNormalsDirtyByVertices(index);
}



void *ActionModelMVScaleVertices::execute(Data *d)
{
	msg_write("scale vert do");
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii)
		m->Vertex[i].pos = pos0 + (old_data[ii] - pos0) * param.x;
	m->SetNormalsDirtyByVertices(index);
	return NULL;
}



void ActionModelMVScaleVertices::redo(Data *d)
{
	execute(d);
}



void ActionModelMVScaleVertices::abort(Data *d)
{
	msg_write("scale vert abort");
	undo(d);
}



void ActionModelMVScaleVertices::set_param(Data *d, const vector & _param)
{
	//msg_write("scale vert set param");
	param = _param;
	execute(d);
}


