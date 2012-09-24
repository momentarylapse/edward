/*
 * ActionModelRotateVertices.h
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELROTATEVERTICES_H_
#define ACTIONMODELROTATEVERTICES_H_

#include "../../../ActionMultiView.h"
class vector;
class DataModel;

class ActionModelRotateVertices: public ActionMultiView
{
public:
	ActionModelRotateVertices(DataModel *d, const vector &_param, const vector &_pos0);
	virtual ~ActionModelRotateVertices();
	string name(){	return "ModelRotateVertices";	}

	void *execute(Data *d);
	void undo(Data *d);
};

#endif /* ACTIONMODELROTATEVERTICES_H_ */
