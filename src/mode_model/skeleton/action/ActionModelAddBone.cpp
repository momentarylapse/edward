/*
 * ActionModelAddBone.cpp
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#include "ActionModelAddBone.h"
#include "../../data/DataModel.h"
#include <cassert>

ActionModelAddBone::ActionModelAddBone(const vec3 &_pos, int _parent) {
	pos = _pos;
	parent = _parent;
}

void *ActionModelAddBone::execute(history::Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModelBone b;
	b.parent = parent;
	b.const_pos = false;
	b.pos = pos;
	b._matrix = mat4::ID;
	b.model = nullptr;
	m->bones.add(b);

	// correct animations
	for (ModelMove &move: m->moves)
		for (ModelFrame &f: move.frames) {
			f.skel_dpos.add(v_0);
			f.skel_ang.add(v_0);
		}

	return &m->bones.back();
}

void ActionModelAddBone::undo(history::Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->bones.pop();

	// correct animations
	for (ModelMove &move: m->moves)
		for (ModelFrame &f: move.frames) {
			f.skel_dpos.pop();
			f.skel_ang.pop();
		}
}


