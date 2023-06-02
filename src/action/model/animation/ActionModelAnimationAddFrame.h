/*
 * ActionModelAnimationAddFrame.h
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELANIMATIONADDFRAME_H_
#define ACTIONMODELANIMATIONADDFRAME_H_

#include "../../Action.h"
#include "../../../data/model/DataModel.h"

class ActionModelAnimationAddFrame: public Action
{
public:
	ActionModelAnimationAddFrame(int index, int frame, const ModelFrame &f);
	string name(){	return "ModelAnimationAddFrame";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index, frame;
	ModelFrame f;
};

#endif /* ACTIONMODELANIMATIONADDFRAME_H_ */
