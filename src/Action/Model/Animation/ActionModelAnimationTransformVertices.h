/*
 * ActionModelAnimationTransformVertices.h
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELANIMATIONTRANSFORMVERTICES_H_
#define ACTIONMODELANIMATIONTRANSFORMVERTICES_H_

#include "../../ActionMultiView.h"
class DataModel;
class vec3;

class ActionModelAnimationTransformVertices: public ActionMultiView
{
public:
	ActionModelAnimationTransformVertices(DataModel *d, int move, int frame);
	string name(){	return "ModelAnimationTransformVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int move, frame;
};

#endif /* ACTIONMODELANIMATIONTRANSFORMVERTICES_H_ */
