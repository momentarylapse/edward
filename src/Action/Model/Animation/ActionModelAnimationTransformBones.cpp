/*
 * ActionModelAnimationTransformBones.cpp
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#include "../../../Data/Model/DataModel.h"
#include "ActionModelAnimationTransformBones.h"

ActionModelAnimationTransformBones::ActionModelAnimationTransformBones(DataModel *d, int _move, int _frame) {
	move = _move;
	frame = _frame;

	// list of selected vertices and save old pos
	foreachi(ModelBone &b, d->bone, i)
		if (b.is_selected){
			index.add(i);
			old_data.add(d->move[move].frame[frame].skel_ang[i]);
			old_dpos.add(d->move[move].frame[frame].skel_dpos[i]);
		}
}

void *ActionModelAnimationTransformBones::execute(Data *d) {
	auto m = dynamic_cast<DataModel*>(d);
	auto q = quaternion::rotation_m( mat);
	foreachi(int i, index, ii){
		m->move[move].frame[frame].skel_ang[i] = (q * quaternion::rotation(old_data[ii])).get_angles();
		if (m->bone[i].parent < 0)
			m->move[move].frame[frame].skel_dpos[i] = mat * (old_dpos[ii] + m->bone[i].pos) - m->bone[i].pos; //+ dpos;
	}
	return nullptr;
}



void ActionModelAnimationTransformBones::undo(Data *d) {
	auto m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii){
		m->move[move].frame[frame].skel_ang[i] = old_data[ii];
		m->move[move].frame[frame].skel_dpos[i] = old_dpos[ii];
	}
}

