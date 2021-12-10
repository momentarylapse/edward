/*
 * ActionModelDeleteUnusedVertex.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModelDeleteUnusedVertex.h"
#include "../../../../../Data/Model/DataModel.h"
#include "../../../../../Data/Model/ModelMesh.h"
#include "../../../../../Edward.h"
#include "../../../../../MultiView/MultiView.h"
#include "../../../../../y/components/Animator.h"
#include <assert.h>


// can only delete lonely vertices!

ActionModelDeleteUnusedVertex::ActionModelDeleteUnusedVertex(int _vertex) {
	vertex = _vertex;
}

void ActionModelDeleteUnusedVertex::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->edit_mesh->add_vertex(pos, bone, bone_weight, normal_mode, vertex);

	// correct animations
	int i = 0;
	for (ModelMove &mv: m->move)
		if (mv.type == AnimationType::VERTEX){
			for (ModelFrame &f: mv.frame)
				f.vertex_dpos.insert(move[i ++], vertex);
		}


	// fx
	for (ModelEffect &f: m->fx)
		if (f.vertex >= vertex)
			f.vertex ++;
	foreachib(ModelEffect &f, fx, i)
		m->fx.insert(f, fx_index[i]);
}



void *ActionModelDeleteUnusedVertex::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	auto &v = m->edit_mesh->vertex[vertex];
	assert(v.ref_count == 0);

	// save old data
	pos = v.pos;
	normal_mode = v.normal_mode;
	bone = v.bone_index;
	bone_weight = v.bone_weight;

	// move data
	move.clear();
	for (ModelMove &mv: m->move)
		if (mv.type == AnimationType::VERTEX) {
			for (ModelFrame &f: mv.frame) {
				move.add(f.vertex_dpos[vertex]);
				f.vertex_dpos.erase(vertex);
			}
		}

	// fx
	fx.clear();
	fx_index.clear();
	foreachib(ModelEffect &f, m->fx, i)
		if (f.vertex == vertex) {
			fx.add(f);
			fx_index.add(i);
			m->fx.erase(i);
			_foreach_it_.update(); // TODO
		} else if (f.vertex > vertex) {
			f.vertex --;
		}

	// erase
	m->edit_mesh->remove_lonely_vertex(vertex);
	return NULL;
}


