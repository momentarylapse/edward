/*
 * ActionModelAnimationSetData.h
 *
 *  Created on: 17.12.2014
 *      Author: michi
 */

#ifndef SRC_ACTION_MODEL_ANIMATION_ACTIONMODELANIMATIONSETDATA_H_
#define SRC_ACTION_MODEL_ANIMATION_ACTIONMODELANIMATIONSETDATA_H_

#include "../../Action.h"

class ActionModelAnimationSetData: public Action
{
public:
	ActionModelAnimationSetData(int index, const string &name, float fps_const, float fps_factor);
	string name(){ return "ModelAnimationSetData"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
	string mname;
	float fps_const;
	float fps_factor;
};

#endif /* SRC_ACTION_MODEL_ANIMATION_ACTIONMODELANIMATIONSETDATA_H_ */
