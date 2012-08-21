/*
 * ActionModelAttachVerticesToBone.cpp
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#include "ActionModelAttachVerticesToBone.h"
#include "../../../Data/Model/DataModel.h"

ActionModelAttachVerticesToBone::ActionModelAttachVerticesToBone(
		Array<int>& _index, int _bone_index)
{
	index = _index;
	bone_index = _bone_index;
}

ActionModelAttachVerticesToBone::~ActionModelAttachVerticesToBone()
{
}

void* ActionModelAttachVerticesToBone::execute(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// save old bone indices
	old_bone.clear();
	foreach(index, i)
		old_bone.add(m->Vertex[i].BoneIndex);

	// apply
	foreach(index, i)
		m->Vertex[i].BoneIndex = bone_index;
	return NULL;
}

void ActionModelAttachVerticesToBone::undo(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// restore old bone indices
	foreachi(index, i, ii)
		m->Vertex[i].BoneIndex = old_bone[ii];
}

