/*
 * ActionModelDeleteBone.cpp
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#include "ActionModelDeleteBone.h"
#include "../../../Data/Model/DataModel.h"

ActionModelDeleteBone::ActionModelDeleteBone(int _index)
{
	index = _index;
}



void *ActionModelDeleteBone::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModelBone &b = m->bone[index];
	pos = b.pos;
	parent = b.parent;
	filename = b.model_file;
	model = b.model;
	child.clear();

	// correct the rest of the skeleton
	foreachi(ModelBone &bb, m->bone, i)
		if (i != index){
			// child -> save and make root
			if (bb.parent == index){
				child.add(i);
				bb.parent = -1;
			}

			// reference > index -> shift
			if (bb.parent > index)
				bb.parent --;
		}

	// save + correct animations
	move_dpos.clear();
	move_ang.clear();
	for (ModelMove &move: m->move)
		for (ModelFrame &f: move.frame){
			move_dpos.add(f.skel_dpos[index]);
			f.skel_dpos.erase(index);
			move_ang.add(f.skel_ang[index]);
			f.skel_ang.erase(index);
		}

	// save + correct vertices
	vertex.clear();
	foreachi(ModelVertex &v, m->vertex, vi)
		if (v.bone_index == index){
			v.bone_index = -1;
			vertex.add(vi);
		}else if (v.bone_index > index)
			v.bone_index --;

	m->bone.erase(index);
	return NULL;
}



void ActionModelDeleteBone::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModelBone b;
	b.parent = parent;
	b.const_pos = false;
	b.pos = pos;
	b._matrix = m_id;
	b.model_file = filename;
	b.model = (Model*)model;
	b.view_stage = 0;
	m->bone.insert(b, index);

	// correct skeleton
	foreachi(ModelBone &bb, m->bone, i)
		if (i != index){
			if (bb.parent >= index)
				bb.parent ++;
		}
	for (int c: child){
		m->bone[c].parent = index;
	}

	// correct animations
	int fi = 0;
	for (ModelMove &move: m->move)
		for (ModelFrame &f: move.frame){
			f.skel_dpos.insert(move_dpos[fi], index);
			f.skel_ang.insert(move_ang[fi], index);
			fi ++;
		}

	// correct vertices
	for (ModelVertex &v: m->vertex)
		if (v.bone_index >= index)
			v.bone_index ++;
	for (int vi: vertex)
		m->vertex[vi].bone_index = index;
}


