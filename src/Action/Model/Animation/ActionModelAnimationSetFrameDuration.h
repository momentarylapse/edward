/*
 * ActionModelAnimationSetFrameDuration.h
 *
 *  Created on: 25.12.2014
 *      Author: michi
 */

#ifndef SRC_ACTION_MODEL_ANIMATION_ACTIONMODELANIMATIONSETFRAMEDURATION_H_
#define SRC_ACTION_MODEL_ANIMATION_ACTIONMODELANIMATIONSETFRAMEDURATION_H_

#include "../../Action.h"
#include "../../../Data/Model/DataModel.h"

class ActionModelAnimationSetFrameDuration: public Action
{
public:
	ActionModelAnimationSetFrameDuration(int index, int frame, float duration);
	string name(){ return "ModelAnimationSetFrameDuration"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index, frame;
	float duration;
};

#endif /* SRC_ACTION_MODEL_ANIMATION_ACTIONMODELANIMATIONSETFRAMEDURATION_H_ */
