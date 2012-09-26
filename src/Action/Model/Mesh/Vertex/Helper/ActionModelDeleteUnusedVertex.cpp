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
	DataModel *m = dynamic_cast<DataModel*>(d);

	// new vertex
	ModelVertex vv;
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
	foreach(ModelMove &mv, m->Move)
		if (mv.Type == MoveTypeVertex){
			foreach(ModelFrame &f, mv.Frame)
				f.VertexDPos[vertex] = move[i ++];
		}


	// correct references
	foreach(ModelSurface &s, m->Surface){
		foreach(ModelPolygon &t, s.Polygon)
			for (int k=0;k<t.Side.num;k++)
				if (t.Side[k].Vertex >= vertex)
					t.Side[k].Vertex ++;
		foreach(ModelEdge &e, s.Edge)
			for (int k=0;k<2;k++)
				if (e.Vertex[k] >= vertex)
					e.Vertex[k] ++;
		foreach(int &v, s.Vertex)
			if (v >= vertex)
				v ++;
	}

	// fx
	foreach(ModelEffect &f, m->Fx)
		if (f.Vertex >= vertex)
			f.Vertex ++;
	foreachib(ModelEffect &f, fx, i)
		m->Fx.insert(f, fx_index[i]);
}



void *ActionModelDeleteUnusedVertex::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(m->Vertex[vertex].RefCount == 0);
	assert(m->Vertex[vertex].Surface < 0);

	// save old data
	pos = m->Vertex[vertex].pos;
	normal_mode = m->Vertex[vertex].NormalMode;
	bone = m->Vertex[vertex].BoneIndex;

	// move data
	move.clear();
	foreach(ModelMove &mv, m->Move)
		if (mv.Type == MoveTypeVertex){
			foreach(ModelFrame &f, mv.Frame)
				move.add(f.VertexDPos[vertex]);
		}

	// correct references
	foreach(ModelSurface &s, m->Surface){
		foreach(ModelPolygon &t, s.Polygon)
			for (int k=0;k<t.Side.num;k++)
				if (t.Side[k].Vertex > vertex)
					t.Side[k].Vertex --;
		foreach(ModelEdge &e, s.Edge)
			for (int k=0;k<2;k++)
				if (e.Vertex[k] > vertex)
					e.Vertex[k] --;
		foreach(int &v, s.Vertex)
			if (v > vertex)
				v --;
	}

	// fx
	fx.clear();
	fx_index.clear();
	foreachib(ModelEffect &f, m->Fx, i)
		if (f.Vertex == vertex){
			fx.add(f);
			fx_index.add(i);
			m->Fx.erase(i);
			_foreach_it_.update(); // TODO
		}else if (f.Vertex > vertex)
			f.Vertex --;

	// erase
	m->Vertex.erase(vertex);
	return NULL;
}


