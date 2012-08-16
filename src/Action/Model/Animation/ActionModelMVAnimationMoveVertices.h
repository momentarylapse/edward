/*
 * ActionModelMVAnimationMoveVertices.h
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMVANIMATIONMOVEVERTICES_H_
#define ACTIONMODELMVANIMATIONMOVEVERTICES_H_

#include "../../ActionMultiView.h"

class ActionModelMVAnimationMoveVertices: public ActionMultiView
{
public:
	ActionModelMVAnimationMoveVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelMVAnimationMoveVertices();
	string name(){	return "ModelAnimationMoveVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int move, frame;
};

#endif /* ACTIONMODELMVANIMATIONMOVEVERTICES_H_ */
