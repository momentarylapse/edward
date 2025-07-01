/*
 * ActionModelDeleteBone.cpp
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#include "ActionModelDeleteBone.h"
#include "../../data/DataModel.h"
#include "../../data/ModelMesh.h"

ActionModelDeleteBone::ActionModelDeleteBone(int _index) {
	index = _index;
}



void *ActionModelDeleteBone::execute(Data *d) {
	auto m = dynamic_cast<DataModel*>(d);
	ModelBone &b = m->bones[index];
	pos = b.pos;
	parent = b.parent;
	filename = b.model_file;
	model = b.model;
	child.clear();

	// correct the rest of the skeleton
	foreachi(ModelBone &bb, m->bones, i)
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
	for (ModelMove &move: m->moves)
		for (ModelFrame &f: move.frames){
			move_dpos.add(f.skel_dpos[index]);
			f.skel_dpos.erase(index);
			move_ang.add(f.skel_ang[index]);
			f.skel_ang.erase(index);
		}

	// save + correct vertices
	vertex.clear();
	foreachi(auto &v, m->editing_mesh->vertices, vi){
		bool found = false;
		if ((v.bone_index.i == index) and (v.bone_weight.x > 0)) {
			v.bone_weight.x = 0;
			found = true;
		} else if ((v.bone_index.j == index) and (v.bone_weight.y > 0)) {
			v.bone_weight.y = 0;
			found = true;
		} else if ((v.bone_index.k == index) and (v.bone_weight.z > 0)) {
			v.bone_weight.z = 0;
			found = true;
		} else if ((v.bone_index.l == index) and (v.bone_weight.w > 0)) {
			v.bone_weight.w = 0;
			found = true;
		}

		if (found) {
			vertex.add(vi);
			vertex_bone.add(v.bone_index);
			vertex_bone_weight.add(v.bone_weight);
			if (v.bone_weight.length() > 0) {
				v.bone_weight.normalize();
			} else {
				v.bone_weight = {1,0,0,0};
				v.bone_index = {0,0,0,0};
			}
		}
	}

	// shift vertex references
	foreachi(auto &v, m->editing_mesh->vertices, vi) {
		if (v.bone_index.i > index)
			v.bone_index.i --;
		if (v.bone_index.j > index)
			v.bone_index.j --;
		if (v.bone_index.k > index)
			v.bone_index.k --;
		if (v.bone_index.l > index)
			v.bone_index.l --;
	}

	m->bones.erase(index);
	return nullptr;
}



void ActionModelDeleteBone::undo(Data *d) {
	auto m = dynamic_cast<DataModel*>(d);
	ModelBone b;
	b.parent = parent;
	b.const_pos = false;
	b.pos = pos;
	b._matrix = mat4::ID;
	b.model_file = filename;
	b.model = (Model*)model;
	m->bones.insert(b, index);

	// correct skeleton
	foreachi(ModelBone &bb, m->bones, i)
		if (i != index){
			if (bb.parent >= index)
				bb.parent ++;
		}
	for (int c: child){
		m->bones[c].parent = index;
	}

	// correct animations
	int fi = 0;
	for (ModelMove &move: m->moves)
		for (ModelFrame &f: move.frames){
			f.skel_dpos.insert(move_dpos[fi], index);
			f.skel_ang.insert(move_ang[fi], index);
			fi ++;
		}

	// correct vertices
	for (auto &v: m->editing_mesh->vertices) {
		if (v.bone_index.i >= index)
			v.bone_index.i ++;
		if (v.bone_index.j >= index)
			v.bone_index.j ++;
		if (v.bone_index.k >= index)
			v.bone_index.k ++;
		if (v.bone_index.l >= index)
			v.bone_index.l ++;
	}
	foreachi (int vi, vertex, i) {
		m->editing_mesh->vertices[vi].bone_index = vertex_bone[i];
		m->editing_mesh->vertices[vi].bone_weight = vertex_bone_weight[i];
	}
}


