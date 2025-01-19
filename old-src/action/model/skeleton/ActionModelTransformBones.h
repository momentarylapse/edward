/*
 * ActionModelTransformBones.h
 *
 *  Created on: 18.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELTRANSFORMBONES_H_
#define ACTIONMODELTRANSFORMBONES_H_

#include "../../ActionMultiView.h"
class DataModel;
class vec3;

class ActionModelTransformBones: public ActionMultiView
{
public:
	ActionModelTransformBones(DataModel *d);
	string name(){	return "ModelTransformBones";	}

	void *execute(Data *d);
	void undo(Data *d);
};

#endif /* ACTIONMODELTRANSFORMBONES_H_ */
