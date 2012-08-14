/*
 * ActionModelMVAnimationRotateBones.h
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMVANIMATIONROTATEBONES_H_
#define ACTIONMODELMVANIMATIONROTATEBONES_H_

#include "../../ActionMultiView.h"

class ActionModelMVAnimationRotateBones: public ActionMultiView
{
public:
	ActionModelMVAnimationRotateBones(Data *d, const vector &_pos0);
	virtual ~ActionModelMVAnimationRotateBones();
	string name(){	return "ModelAnimationRotateBones";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int move, frame;
};

#endif /* ACTIONMODELMVANIMATIONROTATEBONES_H_ */
