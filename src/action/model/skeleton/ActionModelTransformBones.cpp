/*
 * ActionModelTransformBones.cpp
 *
 *  Created on: 18.03.2012
 *      Author: michi
 */

#include "../../../data/model/DataModel.h"
#include "../../../lib/math/math.h"
#include "ActionModelTransformBones.h"

ActionModelTransformBones::ActionModelTransformBones(DataModel *d) :
	ActionMultiView()
{
	// list of selected vertices and save old pos
	foreachi(ModelBone &b, d->bone, i)
		if (b.is_selected){
			index.add(i);
			old_data.add(d->bone[i].pos);
		}
}

void *ActionModelTransformBones::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->bone[i].pos = mat * old_data[ii];
	return NULL;
}



void ActionModelTransformBones::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	foreachi(int i, index, ii)
		m->bone[i].pos = old_data[ii];
}


