/*
 * ActionModelAnimationTransformBones.h
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#ifndef ACTIONMODELANIMATIONTRANSFORMBONES_H_
#define ACTIONMODELANIMATIONTRANSFORMBONES_H_

#include "../../ActionMultiView.h"
class DataModel;
class vector;

class ActionModelAnimationTransformBones: public ActionMultiView
{
public:
	ActionModelAnimationTransformBones(DataModel *d, int move, int frame);
	string name(){	return "ModelAnimationTransformBones";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int move, frame;
	Array<vector> old_dpos;
};

#endif /* ACTIONMODELANIMATIONTRANSFORMBONES_H_ */
