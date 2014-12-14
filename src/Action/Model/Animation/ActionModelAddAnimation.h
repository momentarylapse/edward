/*
 * ActionModelAddAnimation.h
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDANIMATION_H_
#define ACTIONMODELADDANIMATION_H_

#include "../../Action.h"

class ActionModelAddAnimation: public Action
{
public:
	ActionModelAddAnimation(int _index, int _type);
	string name(){	return "ModelAddAnimation";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index, type;
};

#endif /* ACTIONMODELADDANIMATION_H_ */
