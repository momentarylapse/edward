/*
 * ActionModelMVAnimationRotateBones.cpp
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#include "ActionModelMVAnimationRotateBones.h"
#include "../../../Data/Model/DataModel.h"

ActionModelMVAnimationRotateBones::ActionModelMVAnimationRotateBones(Data *d, const vector &_pos0) :
	ActionMultiView(d, _pos0)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	move = m->CurrentMove;
	frame = m->CurrentFrame;

	// list of selected vertices and save old pos
	foreachi(m->Bone, b, i)
		if (b.is_selected){
			index.add(i);
			old_data.add(m->Move[move].Frame[frame].SkelAng[i]);
		}
}

ActionModelMVAnimationRotateBones::~ActionModelMVAnimationRotateBones()
{
}

void *ActionModelMVAnimationRotateBones::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii)
		m->Move[move].Frame[frame].SkelAng[i] = VecAngAdd(old_data[ii], param);
	m->UpdateAnimation();
	return NULL;
}



void ActionModelMVAnimationRotateBones::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii)
		m->Move[move].Frame[frame].SkelAng[i] = old_data[ii];
	m->UpdateAnimation();
}

