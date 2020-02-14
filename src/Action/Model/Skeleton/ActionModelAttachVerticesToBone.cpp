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
	for (int i: index)
		old_bone.add(m->edit_mesh->vertex[i].bone_index);

	// apply
	for (int i: index)
		m->edit_mesh->vertex[i].bone_index = bone_index;
	return NULL;
}

void ActionModelAttachVerticesToBone::undo(Data* d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	// restore old bone indices
	foreachi(int i, index, ii)
		m->edit_mesh->vertex[i].bone_index = old_bone[ii];
}


