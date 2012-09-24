/*
 * ActionModelAnimationMoveVertices.h
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELANIMATIONMOVEVERTICES_H_
#define ACTIONMODELANIMATIONMOVEVERTICES_H_

#include "../../ActionMultiView.h"
class DataModel;
class vector;

class ActionModelAnimationMoveVertices: public ActionMultiView
{
public:
	ActionModelAnimationMoveVertices(DataModel *d, const vector &_param, const vector &_pos0);
	virtual ~ActionModelAnimationMoveVertices();
	string name(){	return "ModelAnimationMoveVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int move, frame;
};

#endif /* ACTIONMODELANIMATIONMOVEVERTICES_H_ */
