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
#include <assert.h>

ActionModelAddVertex::ActionModelAddVertex(const vector &_pos, int _bone_index, int _normal_mode) {
	pos = _pos;
	bone_index = _bone_index;
	normal_mode = _normal_mode;
	if (normal_mode < 0)
		normal_mode = NORMAL_MODE_ANGULAR;
}



void *ActionModelAddVertex::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->mesh->add_vertex(pos, bone_index, normal_mode);

	// correct animations
	for (ModelMove &move: m->move) {
		if (move.type == MOVE_TYPE_VERTEX) {
			for (ModelFrame &f: move.frame)
				f.vertex_dpos.resize(m->mesh->vertex.num);
		}
	}
	return &m->mesh->vertex.back();
}



void ActionModelAddVertex::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(m->mesh->vertex.back().ref_count == 0);

	// delete
	m->mesh->vertex.pop();

	// correct animations
	for (ModelMove &move: m->move)
		if (move.type == MOVE_TYPE_VERTEX) {
			for (ModelFrame &f: move.frame)
				f.vertex_dpos.resize(m->mesh->vertex.num);
		}
}

