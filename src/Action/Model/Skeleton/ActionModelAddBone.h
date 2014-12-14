/*
 * ActionModelAddBone.h
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDBONE_H_
#define ACTIONMODELADDBONE_H_

#include "../../Action.h"
#include "../../../lib/math/math.h"

class ActionModelAddBone: public Action
{
public:
	ActionModelAddBone(const vector &_pos, int _parent);
	string name(){	return "ModelAddBone";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	vector pos;
	int parent;
};

#endif /* ACTIONMODELADDBONE_H_ */
