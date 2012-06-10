/*
 * ActionModelDeleteUnusedVertex.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModelDeleteUnusedVertex.h"
#include "../../../../../Data/Model/DataModel.h"
#include <assert.h>


// can only delete lonely vertices!

ActionModelDeleteUnusedVertex::ActionModelDeleteUnusedVertex(int _vertex)
{
	vertex = _vertex;
}

ActionModelDeleteUnusedVertex::~ActionModelDeleteUnusedVertex()
{
}

void ActionModelDeleteUnusedVertex::undo(Data *d)
{
	msg_write("__del vertex undo");
	DataModel *m = dynamic_cast<DataModel*>(d);

	// new vertex
	ModeModelVertex vv;
	vv.pos = pos;
	vv.NormalMode = normal_mode;
	vv.BoneIndex = bone;
	vv.is_selected = false;
	vv.is_special = false;
	vv.view_stage = m->ViewStage;
	vv.RefCount = 0;
	vv.Surface = -1;
	m->Vertex.insert(vv, vertex);

	// correct animations
	int i = 0;
	foreach(m->Move, mv)
		if (mv.Type == MoveTypeVertex)
			foreach(mv.Frame, f)
				f.VertexDPos[vertex] = move[i ++];


	// correct references
	foreach(m->Surface, s){
		foreach(s.Triangle, t)
			for (int k=0;k<3;k++)
				if (t.Vertex[k] >= vertex)
					t.Vertex[k] ++;
		foreach(s.Edge, e)
			for (int k=0;k<2;k++)
				if (e.Vertex[k] >= vertex)
					e.Vertex[k] ++;
		foreach(s.Vertex, v)
			if (v >= vertex)
				v ++;
	}
}



void *ActionModelDeleteUnusedVertex::execute(Data *d)
{
	msg_write("__del vertex do");
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(m->Vertex[vertex].RefCount == 0);
	assert(m->Vertex[vertex].Surface < 0);

	// save old data
	pos = m->Vertex[vertex].pos;
	normal_mode = m->Vertex[vertex].NormalMode;
	bone = m->Vertex[vertex].BoneIndex;

	// move data
	move.clear();
	foreach(m->Move, mv)
		if (mv.Type == MoveTypeVertex)
			foreach(mv.Frame, f)
				move.add(f.VertexDPos[vertex]);

	// correct references
	foreach(m->Surface, s){
		foreach(s.Triangle, t)
			for (int k=0;k<3;k++)
				if (t.Vertex[k] > vertex)
					t.Vertex[k] --;
		foreach(s.Edge, e)
			for (int k=0;k<2;k++)
				if (e.Vertex[k] > vertex)
					e.Vertex[k] --;
		foreach(s.Vertex, v)
			if (v > vertex)
				v --;
	}

	// erase
	m->Vertex.erase(vertex);
	return NULL;
}


