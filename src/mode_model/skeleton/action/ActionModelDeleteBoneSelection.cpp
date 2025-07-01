/*
 * ActionModelDeleteBoneSelection.cpp
 *
 *  Created on: 20.03.2012
 *      Author: michi
 */

#include "ActionModelDeleteBoneSelection.h"
#include "ActionModelDeleteBone.h"

ActionModelDeleteBoneSelection::ActionModelDeleteBoneSelection(DataModel *m, const base::set<int>& sel) {
	foreachib(ModelBone &b, m->bones, bi)
		if (sel.contains(bi))
			addSubAction(new ActionModelDeleteBone(bi), m);
}
