/*
 * ActionModelAttachVerticesToBone.cpp
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#include "ActionModelAttachVerticesToBone.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Data/Model/ModelMesh.h"

ActionModelAttachVerticesToBone::ActionModelAttachVerticesToBone(const Array<int> &_index, int _bone_index) {
	index = _index;
	bone_index = _bone_index;
}

void* ActionModelAttachVerticesToBone::execute(Data* d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	// save old bone indices
	old_bone.clear();
	for (int i: index) {
		old_bone.add(m->edit_mesh->vertex[i].bone_index);
		old_bone_weight.add(m->edit_mesh->vertex[i].bone_weight);
	}

	// apply
	for (int i: index) {
		m->edit_mesh->vertex[i].bone_index = {bone_index, 0, 0, 0};
		m->edit_mesh->vertex[i].bone_weight = {1, 0, 0, 0};
	}
	return NULL;
}

void ActionModelAttachVerticesToBone::undo(Data* d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	// restore old bone indices
	foreachi(int i, index, ii) {
		m->edit_mesh->vertex[i].bone_index = old_bone[ii];
		m->edit_mesh->vertex[i].bone_weight = old_bone_weight[ii];
	}
}


