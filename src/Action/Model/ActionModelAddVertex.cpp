/*
 * ActionAddVertex.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "ActionModelAddVertex.h"
#include "../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelAddVertex::ActionModelAddVertex(const vector &_v, int _normal_mode)
{
	v = _v;
	normal_mode = _normal_mode;
}

ActionModelAddVertex::~ActionModelAddVertex()
{
}



void *ActionModelAddVertex::execute(Data *d)
{
	msg_write("add vertex do");
	DataModel *m = dynamic_cast<DataModel*>(d);

	// new vertex
	ModeModelVertex vv;
	vv.pos = v;
	if (normal_mode < 0){
		if (m->NormalModeAll != NormalModePerVertex)
			vv.NormalMode = m->NormalModeAll;
		else
			vv.NormalMode = NormalModeAngular;
	}else
		vv.NormalMode = normal_mode;
	vv.BoneIndex = 0;
	vv.is_selected = true;
	vv.is_special = false;
	vv.view_stage = m->ViewStage;
	vv.RefCount = 0;
	vv.Surface = -1;
	m->Vertex.add(vv);

	// correct animations
	foreach(m->Move, move)
		if (move.Type == MoveTypeVertex)
			foreach(move.Frame, f)
				f.VertexDPos.resize(m->Vertex.num);
	return &m->Vertex.back();
}



void ActionModelAddVertex::undo(Data *d)
{
	msg_write("add vertex undo");
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(m->Vertex.back().RefCount == 0);
	assert(m->Vertex.back().Surface < 0);

	// correct animations
	foreach(m->Move, move)
		if (move.Type == MoveTypeVertex)
			foreach(move.Frame, f)
				f.VertexDPos.resize(m->Vertex.num);

	// delete
	m->Vertex.pop();
}



void ActionModelAddVertex::redo(Data *d)
{
	execute(d);
}

