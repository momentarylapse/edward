/*
 * ActionModelAddAnimation.h
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDANIMATION_H_
#define ACTIONMODELADDANIMATION_H_

#include "../../Action.h"

enum class AnimationType;

class ActionModelAddAnimation: public Action {
public:
	ActionModelAddAnimation(int index, AnimationType type);
	string name() { return "ModelAddAnimation"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
	AnimationType type;
};

#endif /* ACTIONMODELADDANIMATION_H_ */
