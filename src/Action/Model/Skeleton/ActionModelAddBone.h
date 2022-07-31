/*
 * ActionModelAddBone.h
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDBONE_H_
#define ACTIONMODELADDBONE_H_

#include "../../../lib/math/vec3.h"
#include "../../Action.h"

class ActionModelAddBone: public Action {
public:
	ActionModelAddBone(const vec3 &_pos, int _parent);
	string name(){	return "ModelAddBone";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	vec3 pos;
	int parent;
};

#endif /* ACTIONMODELADDBONE_H_ */
