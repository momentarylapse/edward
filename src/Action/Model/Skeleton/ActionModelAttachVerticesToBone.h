/*
 * ActionModelAttachVerticesToBone.h
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELATTACHVERTICESTOBONE_H_
#define ACTIONMODELATTACHVERTICESTOBONE_H_

#include "../../Action.h"
#include "../../../lib/math/vector.h"

class ActionModelAttachVerticesToBone: public Action {
public:
	ActionModelAttachVerticesToBone(const Array<int> &_index, int _bone_index);
	string name(){	return "ModelAttachVerticesToBone";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int bone_index;
	Array<int> index;
	Array<ivec4> old_bone;
	Array<vec4> old_bone_weight;
};

#endif /* ACTIONMODELATTACHVERTICESTOBONE_H_ */
