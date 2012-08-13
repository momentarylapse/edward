/*
 * ActionModelDeleteAnimation.h
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETEANIMATION_H_
#define ACTIONMODELDELETEANIMATION_H_

#include "../../Action.h"
#include "../../../Data/Model/DataModel.h"

class ActionModelDeleteAnimation: public Action
{
public:
	ActionModelDeleteAnimation(int _index);
	virtual ~ActionModelDeleteAnimation();
	string name(){	return "ModelDeleteAnimation";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
	ModeModelMove animation;
};

#endif /* ACTIONMODELDELETEANIMATION_H_ */
