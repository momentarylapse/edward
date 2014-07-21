/*
 * ActionModelMoveBones.cpp
 *
 *  Created on: 18.03.2012
 *      Author: michi
 */

#include "ActionModelMoveBones.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../lib/math/math.h"

ActionModelMoveBones::ActionModelMoveBones(DataModel *d) :
	ActionMultiView()
{
	// list of selected vertices and save old pos
	foreachi(ModelBone &b, d->Bone, i)
		if (b.is_selected){
			index.add(i);
			old_data.add(d->Bone[i].pos);
		}
}

ActionModelMoveBones::~ActionModelMoveBones()
{
}

void *ActionModelMoveBones::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreach(int i, index){
		vector p0 = v_0;
		if (m->Bone[i].Parent >= 0)
			p0 = m->Bone[m->Bone[i].Parent].pos;
		m->Bone[i].pos = mat * m->Bone[i].pos;
	}
	foreach(ModelBone &b, m->Bone){
		if (b.Parent >= 0)
			b.DeltaPos = b.pos - m->Bone[b.Parent].pos;
		else
			b.DeltaPos = b.pos;
	}
	return NULL;
}



void ActionModelMoveBones::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->Bone[i].pos = old_data[ii];

	foreach(ModelBone &b, m->Bone){
		if (b.Parent >= 0)
			b.DeltaPos = b.pos - m->Bone[b.Parent].pos;
		else
			b.DeltaPos = b.pos;
	}
}


