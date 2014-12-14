/*
 * ActionModelDeleteBoneSelection.cpp
 *
 *  Created on: 20.03.2012
 *      Author: michi
 */

#include "ActionModelDeleteBoneSelection.h"
#include "ActionModelDeleteBone.h"

ActionModelDeleteBoneSelection::ActionModelDeleteBoneSelection(DataModel *m)
{
	foreachib(ModelBone &b, m->Bone, bi)
		if (b.is_selected)
			AddSubAction(new ActionModelDeleteBone(bi), m);
}
