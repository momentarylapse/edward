/*
 * ActionModelAnimationSetBone.cpp
 *
 *  Created on: 28.12.2014
 *      Author: michi
 */

#include <algorithm>
#include "ActionModelAnimationSetBone.h"
#include "../../../data/model/DataModel.h"
#include <assert.h>

ActionModelAnimationSetBone::ActionModelAnimationSetBone(int _move, int _frame, int _bone, const vec3& _dpos, const vec3& _ang)
{
	move = _move;
	frame = _frame;
	bone = _bone;
	dpos = _dpos;
	ang = _ang;
}

void* ActionModelAnimationSetBone::execute(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(move >= 0);
	assert(move < m->move.num);
	ModelMove &mv = m->move[move];
	assert(frame >= 0);
	assert(frame < mv.frame.num);
	ModelFrame &f = mv.frame[frame];
	assert(bone >= 0);
	assert(bone < m->bone.num);

	std::swap(f.skel_dpos[bone], dpos);
	std::swap(f.skel_ang[bone], ang);

	return NULL;
}

void ActionModelAnimationSetBone::undo(Data* d)
{
	execute(d);
}
