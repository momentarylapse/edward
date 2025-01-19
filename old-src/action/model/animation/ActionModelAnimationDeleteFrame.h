/*
 * ActionModelAnimationDeleteFrame.h
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELANIMATIONDELETEFRAME_H_
#define ACTIONMODELANIMATIONDELETEFRAME_H_

#include "../../Action.h"
#include "../../../data/model/DataModel.h"

class ActionModelAnimationDeleteFrame: public Action
{
public:
	ActionModelAnimationDeleteFrame(int _index, int _frame);
	string name(){	return "ModelAnimationDeleteFrame";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	bool aborted;
	int index, frame;
	ModelFrame old_frame;
};

#endif /* ACTIONMODELANIMATIONDELETEFRAME_H_ */
