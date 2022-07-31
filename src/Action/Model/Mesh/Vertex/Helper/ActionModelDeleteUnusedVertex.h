/*
 * ActionModelDeleteUnusedVertex.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETEUNUSEDVERTEX_H_
#define ACTIONMODELDELETEUNUSEDVERTEX_H_

#include "../../../../Action.h"
#include "../../../../../Data/Model/DataModel.h" // can't we use Array<ModelEffect> ?!?!?
#include "../../../../../lib/math/vec3.h"

class ModelEffect;

class ActionModelDeleteUnusedVertex: public Action {
public:
	ActionModelDeleteUnusedVertex(int _vertex);
	string name(){	return "ModelDeleteUnusedVertex";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int vertex;
	vec3 pos;
	ivec4 bone;
	vec4 bone_weight;
	int normal_mode;
	Array<vec3> move;
	Array<ModelEffect> fx;
	Array<int> fx_index;
};

#endif /* ACTIONMODELDELETEUNUSEDVERTEX_H_ */
