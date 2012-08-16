/*
 * ActionModelMVAnimationRotateVertices.h
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMVANIMATIONROTATEVERTICES_H_
#define ACTIONMODELMVANIMATIONROTATEVERTICES_H_

#include "../../ActionMultiView.h"

class ActionModelMVAnimationRotateVertices: public ActionMultiView
{
public:
	ActionModelMVAnimationRotateVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelMVAnimationRotateVertices();
	string name(){	return "ModelAnimationRotateVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int move, frame;
};

#endif /* ACTIONMODELMVANIMATIONROTATEVERTICES_H_ */
