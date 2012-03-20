/*
 * ActionModelAddBone.cpp
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#include "ActionModelAddBone.h"
#include "../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelAddBone::ActionModelAddBone(const vector &_pos, int _parent)
{
	pos = _pos;
	parent = _parent;
}

ActionModelAddBone::~ActionModelAddBone()
{
}

void *ActionModelAddBone::execute(Data *d)
{
	msg_write("add bone do");
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModeModelSkeletonBone b;
	b.Parent = parent;
	b.ConstPos = false;
	b.pos = pos;
	if (parent >= 0)
		b.DeltaPos = pos - m->Bone[parent].pos;
	else
		b.DeltaPos = pos;
	b.Matrix = m_id;
	b.model = NULL;
	b.view_stage = 0;
	b.is_special = b.is_selected = false;
	m->Bone.add(b);

	// correct animations
	foreach(m->Move, move)
		foreach(move.Frame, f){
			f.SkelDPos.add(v0);
			f.SkelAng.add(v0);
		}

	return &m->Bone.back();
}



void ActionModelAddBone::redo(Data *d)
{
	execute(d);
}



void ActionModelAddBone::undo(Data *d)
{
	msg_write("add bone undo");
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->Bone.pop();

	// correct animations
	foreach(m->Move, move)
		foreach(move.Frame, f){
			f.SkelDPos.pop();
			f.SkelAng.pop();
		}
}


