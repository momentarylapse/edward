/*
 * ActionModelAnimationSetBone.h
 *
 *  Created on: 28.12.2014
 *      Author: michi
 */

#ifndef SRC_ACTION_MODEL_ANIMATION_ACTIONMODELANIMATIONSETBONE_H_
#define SRC_ACTION_MODEL_ANIMATION_ACTIONMODELANIMATIONSETBONE_H_

#include "../../../lib/math/vec3.h"
#include "../../Action.h"
class DataModel;

class ActionModelAnimationSetBone: public Action {
public:
	ActionModelAnimationSetBone(int move, int frame, int bone, const vec3 &dpos, const vec3 &ang);
	string name(){ return "ModelAnimationSetBone"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	int move, frame, bone;
	vec3 dpos;
	vec3 ang;
};

#endif /* SRC_ACTION_MODEL_ANIMATION_ACTIONMODELANIMATIONSETBONE_H_ */
