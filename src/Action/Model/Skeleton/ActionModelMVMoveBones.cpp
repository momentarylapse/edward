/*
 * ActionModelMVMoveBones.cpp
 *
 *  Created on: 18.03.2012
 *      Author: michi
 */

#include "ActionModelMVMoveBones.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../lib/file/file.h"
#include "../../../lib/types/types.h"

ActionModelMVMoveBones::ActionModelMVMoveBones(Data *d, const vector &_pos0) :
	ActionMultiView(d, _pos0)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// list of selected vertices and save old pos
	foreachi(m->Bone, b, i)
		if (b.is_selected){
			index.add(i);
			old_data.add(m->Bone[i].pos);
		}
}

ActionModelMVMoveBones::~ActionModelMVMoveBones()
{
}

void *ActionModelMVMoveBones::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii){
		m->Bone[i].pos = old_data[ii] + param;
		if (m->Bone[i].Parent >= 0)
			m->Bone[i].DeltaPos = m->Bone[i].pos - m->Bone[m->Bone[i].Parent].pos;
		else
			m->Bone[i].DeltaPos = m->Bone[i].pos;
	}
	return NULL;
}



void ActionModelMVMoveBones::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(index, i, ii){
		m->Bone[i].pos = old_data[ii];
		if (m->Bone[i].Parent >= 0)
			m->Bone[i].DeltaPos = m->Bone[i].pos - m->Bone[m->Bone[i].Parent].pos;
		else
			m->Bone[i].DeltaPos = m->Bone[i].pos;
	}
}


