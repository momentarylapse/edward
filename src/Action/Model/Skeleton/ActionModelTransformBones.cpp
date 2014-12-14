/*
 * ActionModelTransformBones.cpp
 *
 *  Created on: 18.03.2012
 *      Author: michi
 */

#include "../../../Data/Model/DataModel.h"
#include "../../../lib/math/math.h"
#include "ActionModelTransformBones.h"

ActionModelTransformBones::ActionModelTransformBones(DataModel *d) :
	ActionMultiView()
{
	// list of selected vertices and save old pos
	foreachi(ModelBone &b, d->Bone, i)
		if (b.is_selected){
			index.add(i);
			old_data.add(d->Bone[i].pos);
		}
}

void *ActionModelTransformBones::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreach(int i, index){
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



void ActionModelTransformBones::undo(Data *d)
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


