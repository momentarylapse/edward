/*
 * ActionModelDeleteAnimation.h
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETEANIMATION_H_
#define ACTIONMODELDELETEANIMATION_H_

#include "../../Action.h"
#include "../../../data/model/DataModel.h"

class ActionModelDeleteAnimation: public Action
{
public:
	ActionModelDeleteAnimation(int _index);
	string name(){	return "ModelDeleteAnimation";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
	ModelMove animation;
};

#endif /* ACTIONMODELDELETEANIMATION_H_ */
