/*
 * ActionModelReconnectBone.cpp
 *
 *  Created on: 14.12.2014
 *      Author: michi
 */

#include "ActionModelReconnectBone.h"
#include "../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelReconnectBone::ActionModelReconnectBone(int _index, int _parent)
{
	index = _index;
	parent = _parent;
}

void *ActionModelReconnectBone::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index >= 0);
	assert(index < m->Bone.num);
	ModelBone &b = m->Bone[index];

	int t = b.Parent;
	b.Parent = parent;
	parent = t;

	return &b;
}



void ActionModelReconnectBone::undo(Data *d)
{
	execute(d);
}

