/*
 * ActionModelAnimationRotateBones.cpp
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#include "ActionModelAnimationRotateBones.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Mode/Model/Animation/ModeModelAnimation.h"

ActionModelAnimationRotateBones::ActionModelAnimationRotateBones(DataModel *d, const vector &_param, const vector &_pos0) :
	ActionMultiView()
{
	move = mode_model_animation->CurrentMove;
	frame = mode_model_animation->CurrentFrame;

	// list of selected vertices and save old pos
	foreachi(ModelBone &b, d->Bone, i)
		if (b.is_selected){
			index.add(i);
			old_data.add(d->Move[move].Frame[frame].SkelAng[i]);
		}
}

ActionModelAnimationRotateBones::~ActionModelAnimationRotateBones()
{
}

void *ActionModelAnimationRotateBones::execute(Data *d)
{
	/*DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Move[move].Frame[frame].SkelAng[i] = VecAngAdd(old_data[ii], param);*/
	return NULL;
}



void ActionModelAnimationRotateBones::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Move[move].Frame[frame].SkelAng[i] = old_data[ii];
}

