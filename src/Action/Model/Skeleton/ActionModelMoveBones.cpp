/*
 * ActionModelMoveBones.cpp
 *
 *  Created on: 18.03.2012
 *      Author: michi
 */

#include "ActionModelMoveBones.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../lib/file/file.h"
#include "../../../lib/types/types.h"

ActionModelMoveBones::ActionModelMoveBones(Data *d, const vector &_pos0) :
	ActionMultiView(d, _pos0)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// list of selected vertices and save old pos
	foreachi(ModelBone &b, m->Bone, i)
		if (b.is_selected){
			index.add(i);
			old_data.add(m->Bone[i].pos);
		}
}

ActionModelMoveBones::~ActionModelMoveBones()
{
}

void *ActionModelMoveBones::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii){
		m->Bone[i].pos = old_data[ii] + param;
		if (m->Bone[i].Parent >= 0)
			m->Bone[i].DeltaPos = m->Bone[i].pos - m->Bone[m->Bone[i].Parent].pos;
		else
			m->Bone[i].DeltaPos = m->Bone[i].pos;
	}
	return NULL;
}



void ActionModelMoveBones::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii){
		m->Bone[i].pos = old_data[ii];
		if (m->Bone[i].Parent >= 0)
			m->Bone[i].DeltaPos = m->Bone[i].pos - m->Bone[m->Bone[i].Parent].pos;
		else
			m->Bone[i].DeltaPos = m->Bone[i].pos;
	}
}


