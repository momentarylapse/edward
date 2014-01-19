/*
 * ActionModelTransformVertices.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELTRANSFORMVERTICES_H_
#define ACTIONMODELTRANSFORMVERTICES_H_

#include "../../../ActionMultiView.h"
class DataModel;
class vector;

class ActionModelTransformVertices: public ActionMultiView
{
public:
	ActionModelTransformVertices(DataModel *d);
	virtual ~ActionModelTransformVertices();
	string name(){	return "ModelTransformVertices";	}

	void *execute(Data *d);
	void undo(Data *d);
};

#endif /* ACTIONMODELTRANSFORMVERTICES_H_ */
