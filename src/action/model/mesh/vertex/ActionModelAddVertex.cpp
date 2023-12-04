/*
 * ActionAddVertex.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "ActionModelAddVertex.h"
#include "../../../../data/model/DataModel.h"
#include "../../../../data/model/ModelMesh.h"
#include "../../../../data/model/ModelPolygon.h"
#include <y/world/components/Animator.h>
#include <assert.h>

ActionModelAddVertex::ActionModelAddVertex(const vec3 &_pos, const ivec4 &_bone_index, const vec4 &_bone_weight, int _normal_mode) {
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
	m->edit_mesh->_post_vertex_number_change_update();

	return &m->edit_mesh->vertex.back();
}



void ActionModelAddVertex::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(m->edit_mesh->vertex.back().ref_count == 0);

	m->edit_mesh->vertex.pop();
	m->edit_mesh->_post_vertex_number_change_update();
}

