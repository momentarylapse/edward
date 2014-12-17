/*
 * ActionModelAddAnimation.cpp
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#include "ActionModelAddAnimation.h"
#include "../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelAddAnimation::ActionModelAddAnimation(int _index, int _type)
{
	index = _index;
	type = _type;
}

void *ActionModelAddAnimation::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index >= 0);
	if (m->move.num >= index + 1)
		assert(m->move[index].frame.num == 0);

	// add animation "descriptor"
	if (m->move.num < index + 1)
		m->move.resize(index + 1);
	m->move[index].frame.clear();
	m->move[index].name = "animation-"+i2s(index);
	m->move[index].type = type;
	m->move[index].frames_per_sec_const = 2;
	m->move[index].frames_per_sec_factor = 0;

	// add animation data
	ModelFrame f;
	if (type ==MOVE_TYPE_SKELETAL){
		f.skel_ang.resize(m->bone.num);
		f.skel_dpos.resize(m->bone.num);
	}else if (type ==MOVE_TYPE_VERTEX){
		f.vertex_dpos.resize(m->vertex.num);
	}
	m->move[index].frame.add(f);
	return NULL;
}

void ActionModelAddAnimation::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->move[index].frame.clear();
}

