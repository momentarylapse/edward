/*
 * ActionModelAnimationRotateBones.h
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELANIMATIONROTATEBONES_H_
#define ACTIONMODELANIMATIONROTATEBONES_H_

#include "../../ActionMultiView.h"

class ActionModelAnimationRotateBones: public ActionMultiView
{
public:
	ActionModelAnimationRotateBones(Data *d, const vector &_pos0);
	virtual ~ActionModelAnimationRotateBones();
	string name(){	return "ModelAnimationRotateBones";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int move, frame;
};

#endif /* ACTIONMODELANIMATIONROTATEBONES_H_ */
