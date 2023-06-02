/*
 * ActionModelReconnectBone.h
 *
 *  Created on: 14.12.2014
 *      Author: michi
 */

#ifndef SRC_ACTION_MODEL_SKELETON_ACTIONMODELRECONNECTBONE_H_
#define SRC_ACTION_MODEL_SKELETON_ACTIONMODELRECONNECTBONE_H_

#include "../../Action.h"
#include "../../../lib/math/math.h"

class ActionModelReconnectBone: public Action
{
public:
	ActionModelReconnectBone(int _index, int _parent);
	string name(){	return "ModelReconnectBone";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
	int parent;
};

#endif /* SRC_ACTION_MODEL_SKELETON_ACTIONMODELRECONNECTBONE_H_ */
