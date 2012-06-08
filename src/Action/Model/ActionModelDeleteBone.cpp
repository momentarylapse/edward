/*
 * ActionModelDeleteBone.cpp
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#include "ActionModelDeleteBone.h"
#include "../../Data/Model/DataModel.h"

ActionModelDeleteBone::ActionModelDeleteBone(int _index)
{
	index = _index;
}

ActionModelDeleteBone::~ActionModelDeleteBone()
{
}



void *ActionModelDeleteBone::execute(Data *d)
{
	msg_write("del bone do");
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModeModelSkeletonBone &b = m->Bone[index];
	pos = b.DeltaPos;
	parent = b.Parent;
	filename = b.ModelFile;
	model = b.model;
	child.clear();

	// correct the rest of the skeleton
	foreachi(m->Bone, bb, i)
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
	foreach(m->Move, move)
		foreach(move.Frame, f){
			move_dpos.add(f.SkelDPos[index]);
			f.SkelDPos.erase(index);
			move_ang.add(f.SkelAng[index]);
			f.SkelAng.erase(index);
		}

	// save + correct vertices
	vertex.clear();
	foreachi(m->Vertex, v, vi)
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
	msg_write("del bone undo");
	DataModel *m = dynamic_cast<DataModel*>(d);
	ModeModelSkeletonBone b;
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
	foreachi(m->Bone, bb, i)
		if (i != index){
			if (bb.Parent >= index)
				bb.Parent ++;
		}
	foreach(child, c){
		m->Bone[c].Parent = index;
		m->Bone[c].DeltaPos = m->Bone[c].pos - pos;
	}

	// correct animations
	int fi = 0;
	foreach(m->Move, move)
		foreach(move.Frame, f){
			f.SkelDPos.insert(move_dpos[fi], index);
			f.SkelAng.insert(move_ang[fi], index);
			fi ++;
		}

	// correct vertices
	foreach(m->Vertex, v)
		if (v.BoneIndex >= index)
			v.BoneIndex ++;
	foreach(vertex, vi)
		m->Vertex[vi].BoneIndex = index;
}


