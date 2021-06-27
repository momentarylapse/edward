/*
 * ActionModelAnimationSetBone.h
 *
 *  Created on: 28.12.2014
 *      Author: michi
 */

#ifndef SRC_ACTION_MODEL_ANIMATION_ACTIONMODELANIMATIONSETBONE_H_
#define SRC_ACTION_MODEL_ANIMATION_ACTIONMODELANIMATIONSETBONE_H_

#include "../../Action.h"
#include "../../../lib/math/vector.h"
class DataModel;

class ActionModelAnimationSetBone: public Action {
public:
	ActionModelAnimationSetBone(int move, int frame, int bone, const vector &dpos, const vector &ang);
	string name(){ return "ModelAnimationSetBone"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	int move, frame, bone;
	vector dpos;
	vector ang;
};

#endif /* SRC_ACTION_MODEL_ANIMATION_ACTIONMODELANIMATIONSETBONE_H_ */
