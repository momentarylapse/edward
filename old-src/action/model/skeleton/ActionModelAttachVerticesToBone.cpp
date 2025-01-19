/*
 * ActionModelAttachVerticesToBone.cpp
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#include "ActionModelAttachVerticesToBone.h"
#include "../../../data/model/DataModel.h"
#include "../../../data/model/ModelMesh.h"

ActionModelAttachVerticesToBone::ActionModelAttachVerticesToBone(const Array<int> &_index, const Array<ivec4> &_bone, const Array<vec4> &_weight) {
	index = _index;
	bone = _bone;
	weight = _weight;
}

void* ActionModelAttachVerticesToBone::execute(Data* d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreachi (int i, index, ii) {
		std::swap(m->edit_mesh->vertex[i].bone_index, bone[ii]);
		std::swap(m->edit_mesh->vertex[i].bone_weight, weight[ii]);
	}
	return nullptr;
}

void ActionModelAttachVerticesToBone::undo(Data* d) {
	execute(d);
}


