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

ActionModelDeleteBone::~ActionModelDeleteBone()
{
}



void *ActionModelDeleteBone::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModelBone &b = m->Bone[index];
	pos = b.DeltaPos;
	parent = b.Parent;
	filename = b.ModelFile;
	model = b.model;
	child.clear();

	// correct the rest of the skeleton
	foreachi(ModelBone &bb, m->Bone, i)
		if (i != index){
			// child -> save and make root
			if (bb.Parent == index){
				child.add(i);
				bb.Parent = -1;
				bb.DeltaPos = bb.pos;
			}

			// reference > index -> shift
			if (bb.Parent > index)
				bb.Parent --;
		}

	// save + correct animations
	move_dpos.clear();
	move_ang.clear();
	foreach(ModelMove &move, m->Move)
		foreach(ModelFrame &f, move.Frame){
			move_dpos.add(f.SkelDPos[index]);
			f.SkelDPos.erase(index);
			move_ang.add(f.SkelAng[index]);
			f.SkelAng.erase(index);
		}

	// save + correct vertices
	vertex.clear();
	foreachi(ModelVertex &v, m->Vertex, vi)
		if (v.BoneIndex == index){
			v.BoneIndex = -1;
			vertex.add(vi);
		}else if (v.BoneIndex > index)
			v.BoneIndex --;

	m->Bone.erase(index);
	return NULL;
}



void ActionModelDeleteBone::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModelBone b;
	b.Parent = parent;
	b.ConstPos = false;
	b.DeltaPos = pos;
	if (parent >= 0)
		b.pos = pos + m->Bone[parent].pos;
	else
		b.pos = pos;
	b.Matrix = m_id;
	b.ModelFile = filename;
	b.model = (CModel*)model;
	b.view_stage = 0;
	m->Bone.insert(b, index);

	// correct skeleton
	foreachi(ModelBone &bb, m->Bone, i)
		if (i != index){
			if (bb.Parent >= index)
				bb.Parent ++;
		}
	foreach(int c, child){
		m->Bone[c].Parent = index;
		m->Bone[c].DeltaPos = m->Bone[c].pos - pos;
	}

	// correct animations
	int fi = 0;
	foreach(ModelMove &move, m->Move)
		foreach(ModelFrame &f, move.Frame){
			f.SkelDPos.insert(move_dpos[fi], index);
			f.SkelAng.insert(move_ang[fi], index);
			fi ++;
		}

	// correct vertices
	foreach(ModelVertex &v, m->Vertex)
		if (v.BoneIndex >= index)
			v.BoneIndex ++;
	foreach(int vi, vertex)
		m->Vertex[vi].BoneIndex = index;
}


