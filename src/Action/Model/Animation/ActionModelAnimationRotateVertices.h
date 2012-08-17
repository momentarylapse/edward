/*
 * ActionModelAnimationRotateVertices.h
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELANIMATIONROTATEVERTICES_H_
#define ACTIONMODELANIMATIONROTATEVERTICES_H_

#include "../../ActionMultiView.h"

class ActionModelAnimationRotateVertices: public ActionMultiView
{
public:
	ActionModelAnimationRotateVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelAnimationRotateVertices();
	string name(){	return "ModelAnimationRotateVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int move, frame;
};

#endif /* ACTIONMODELANIMATIONROTATEVERTICES_H_ */
