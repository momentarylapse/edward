/*
 * ActionModelAddBone.cpp
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#include "ActionModelAddBone.h"
#include "../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelAddBone::ActionModelAddBone(const vec3 &_pos, int _parent)
{
	pos = _pos;
	parent = _parent;
}

void *ActionModelAddBone::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModelBone b;
	b.parent = parent;
	b.const_pos = false;
	b.pos = pos;
	b._matrix = mat4::ID;
	b.model = NULL;
	m->bone.add(b);

	// correct animations
	for (ModelMove &move: m->move)
		for (ModelFrame &f: move.frame){
			f.skel_dpos.add(v_0);
			f.skel_ang.add(v_0);
		}

	return &m->bone.back();
}



void ActionModelAddBone::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->bone.pop();

	// correct animations
	for (ModelMove &move: m->move)
		for (ModelFrame &f: move.frame){
			f.skel_dpos.pop();
			f.skel_ang.pop();
		}
}


