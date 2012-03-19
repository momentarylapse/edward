/*
 * ActionModelDeleteBone.cpp
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#include "ActionModelDeleteBone.h"
#include "../../Data/Model/DataModel.h"

ActionModelDeleteBone::ActionModelDeleteBone(int _index)
{
	index = _index;
}

ActionModelDeleteBone::~ActionModelDeleteBone()
{
}

void ActionModelDeleteBone::redo(Data *d)
{
	execute(d);
}



void *ActionModelDeleteBone::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModeModelSkeletonBone &b = m->Bone[index];
	pos = b.DeltaPos;
	parent = b.Parent;
	filename = b.ModelFile;
	model = b.model;
	child.clear();

	// correct the rest of the skeleton
	foreachi(m->Bone, bb, i)
		if (i != index){
			// child -> save and make root
			if (bb.Parent == index){
				child.add(i);
				bb.Parent = -1;
				bb.DeltaPos = bb.pos;
			}

			// reference > index -> shift
			if (bb.Parent > index)
				bb.Parent --;
		}
	return NULL;
}



void ActionModelDeleteBone::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModeModelSkeletonBone b;
	b.Parent = parent;
	b.ConstPos = false;
	b.DeltaPos = pos;
	if (parent >= 0)
		b.pos = pos + m->Bone[parent].pos;
	else
		b.pos = pos;
	b.Matrix = m_id;
	b.ModelFile = filename;
	b.model = (CModel*)model;
	b.view_stage = 0;
	m->Bone.insert(b, index);

	// correct skeleton
	foreachi(m->Bone, bb, i)
		if (i != index){
			if (bb.Parent >= index)
				bb.Parent ++;
		}
	foreach(child, c){
		m->Bone[c].Parent = index;
		m->Bone[c].DeltaPos = m->Bone[c].pos - pos;
	}
}


