/*
 * ActionModelDuplicateAnimation.h
 *
 *  Created on: 26.12.2014
 *      Author: michi
 */

#ifndef ACTIONMODELDUPLICATEANIMATION_H_
#define ACTIONMODELDUPLICATEANIMATION_H_

#include "../../Action.h"

class ActionModelDuplicateAnimation: public Action
{
public:
	ActionModelDuplicateAnimation(int source, int target);
	string name(){	return "ModelDuplicateAnimation";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int source, target;
};

#endif /* ACTIONMODELDUPLICATEANIMATION_H_ */
