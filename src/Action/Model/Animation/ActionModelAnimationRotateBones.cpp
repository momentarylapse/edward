/*
 * ActionModelAnimationRotateBones.cpp
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#include "ActionModelAnimationRotateBones.h"
#include "../../../Data/Model/DataModel.h"

ActionModelAnimationRotateBones::ActionModelAnimationRotateBones(Data *d, const vector &_pos0) :
	ActionMultiView(d, _pos0)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	move = m->CurrentMove;
	frame = m->CurrentFrame;

	// list of selected vertices and save old pos
	foreachi(ModelBone &b, m->Bone, i)
		if (b.is_selected){
			index.add(i);
			old_data.add(m->Move[move].Frame[frame].SkelAng[i]);
		}
}

ActionModelAnimationRotateBones::~ActionModelAnimationRotateBones()
{
}

void *ActionModelAnimationRotateBones::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Move[move].Frame[frame].SkelAng[i] = VecAngAdd(old_data[ii], param);
	m->UpdateAnimation();
	return NULL;
}



void ActionModelAnimationRotateBones::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Move[move].Frame[frame].SkelAng[i] = old_data[ii];
	m->UpdateAnimation();
}

