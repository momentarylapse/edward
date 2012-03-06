/*
 * ActionAddVertex.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "ActionModelAddVertex.h"
#include "../../Data/Model/DataModel.h"

ActionModelAddVertex::ActionModelAddVertex(const vector & _v)
{
	v = _v;
}

ActionModelAddVertex::~ActionModelAddVertex()
{
}

bool ActionModelAddVertex::is_atom()
{	return true;	}

bool ActionModelAddVertex::needs_preparation()
{	return false;	}



void *ActionModelAddVertex::execute(Data *d)
{
	msg_write("add vertex do");
	DataModel *m = dynamic_cast<DataModel*>(d);

	// new vertex
	ModeModelVertex vv;
	vv.pos = v;
	vv.NormalMode = NormalModeAngular;
	vv.BoneIndex = 0;
	vv.is_selected = false;
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



void ActionModelAddVertex::prepare(Data *d)
{
}



void ActionModelAddVertex::undo(Data *d)
{
	msg_write("add vertex undo");
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->Vertex.pop();

	// correct animations
	foreach(m->Move, move)
		if (move.Type == MoveTypeVertex)
			foreach(move.Frame, f)
				f.VertexDPos.resize(m->Vertex.num);
}



void ActionModelAddVertex::redo(Data *d)
{
	execute(d);
}

