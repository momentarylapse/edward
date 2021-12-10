/*
 * ActionAddVertex.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "ActionModelAddVertex.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/ModelMesh.h"
#include "../../../../Data/Model/ModelPolygon.h"
#include "../../../../y/components/Animator.h"
#include <assert.h>

ActionModelAddVertex::ActionModelAddVertex(const vector &_pos, const ivec4 &_bone_index, const vec4 &_bone_weight, int _normal_mode) {
	pos = _pos;
	bone_index = _bone_index;
	bone_weight = _bone_weight;
	normal_mode = _normal_mode;
	if (normal_mode < 0)
		normal_mode = NORMAL_MODE_ANGULAR;
}



void *ActionModelAddVertex::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->edit_mesh->add_vertex(pos, bone_index, bone_weight, normal_mode);

	// correct animations
	for (ModelMove &move: m->move) {
		if (move.type == AnimationType::VERTEX) {
			for (ModelFrame &f: move.frame)
				f.vertex_dpos.resize(m->edit_mesh->vertex.num);
		}
	}
	return &m->edit_mesh->vertex.back();
}



void ActionModelAddVertex::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(m->edit_mesh->vertex.back().ref_count == 0);

	// delete
	m->edit_mesh->vertex.pop();

	// correct animations
	for (ModelMove &move: m->move)
		if (move.type == AnimationType::VERTEX) {
			for (ModelFrame &f: move.frame)
				f.vertex_dpos.resize(m->edit_mesh->vertex.num);
		}
}

