/*
 * ActionModelAnimationAddFrame.h
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELANIMATIONADDFRAME_H_
#define ACTIONMODELANIMATIONADDFRAME_H_

#include "../../Action.h"

class ActionModelAnimationAddFrame: public Action
{
public:
	ActionModelAnimationAddFrame(int _index, int _frame);
	string name(){	return "ModelAnimationAddFrame";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index, frame;
};

#endif /* ACTIONMODELANIMATIONADDFRAME_H_ */
