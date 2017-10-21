/*
 * ActionModelDeleteUnusedVertex.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModelDeleteUnusedVertex.h"
#include "../../../../../Data/Model/DataModel.h"
#include "../../../../../Edward.h"
#include "../../../../../MultiView/MultiView.h"
#include <assert.h>


// can only delete lonely vertices!

ActionModelDeleteUnusedVertex::ActionModelDeleteUnusedVertex(int _vertex)
{
	vertex = _vertex;
}

void ActionModelDeleteUnusedVertex::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// new vertex
	ModelVertex vv;
	vv.pos = pos;
	vv.normal_mode = normal_mode;
	vv.bone_index = bone;
	vv.is_selected = false;
	vv.is_special = false;
	vv.view_stage = ed->multi_view_3d->view_stage;
	vv.ref_count = 0;
	vv.surface = -1;
	m->vertex.insert(vv, vertex);

	// correct animations
	int i = 0;
	for (ModelMove &mv: m->move)
		if (mv.type == MOVE_TYPE_VERTEX){
			for (ModelFrame &f: mv.frame)
				f.vertex_dpos.insert(move[i ++], vertex);
		}


	// correct references
	for (ModelSurface &s: m->surface){
		for (ModelPolygon &t: s.polygon)
			for (int k=0;k<t.side.num;k++)
				if (t.side[k].vertex >= vertex)
					t.side[k].vertex ++;
		for (ModelEdge &e: s.edge)
			for (int k=0;k<2;k++)
				if (e.vertex[k] >= vertex)
					e.vertex[k] ++;
		for (int &v: s.vertex)
			if (v >= vertex)
				v ++;
	}

	// fx
	for (ModelEffect &f: m->fx)
		if (f.vertex >= vertex)
			f.vertex ++;
	foreachib(ModelEffect &f, fx, i)
		m->fx.insert(f, fx_index[i]);
}



void *ActionModelDeleteUnusedVertex::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(m->vertex[vertex].ref_count == 0);
	assert(m->vertex[vertex].surface < 0);

	// save old data
	pos = m->vertex[vertex].pos;
	normal_mode = m->vertex[vertex].normal_mode;
	bone = m->vertex[vertex].bone_index;

	// move data
	move.clear();
	for (ModelMove &mv: m->move)
		if (mv.type == MOVE_TYPE_VERTEX){
			for (ModelFrame &f: mv.frame){
				move.add(f.vertex_dpos[vertex]);
				f.vertex_dpos.erase(vertex);
			}
		}

	// correct references
	for (ModelSurface &s: m->surface){
		for (ModelPolygon &t: s.polygon)
			for (int k=0;k<t.side.num;k++)
				if (t.side[k].vertex > vertex)
					t.side[k].vertex --;
		for (ModelEdge &e: s.edge)
			for (int k=0;k<2;k++)
				if (e.vertex[k] > vertex)
					e.vertex[k] --;
		for (int &v: s.vertex)
			if (v > vertex)
				v --;
	}

	// fx
	fx.clear();
	fx_index.clear();
	foreachib(ModelEffect &f, m->fx, i)
		if (f.vertex == vertex){
			fx.add(f);
			fx_index.add(i);
			m->fx.erase(i);
			_foreach_it_.update(); // TODO
		}else if (f.vertex > vertex)
			f.vertex --;

	// erase
	m->vertex.erase(vertex);
	return NULL;
}


