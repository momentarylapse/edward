/*
 * ActionModelMoveVertices.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMOVEVERTICES_H_
#define ACTIONMODELMOVEVERTICES_H_

#include "../../../ActionMultiView.h"
class DataModel;
class vector;

class ActionModelMoveVertices: public ActionMultiView
{
public:
	ActionModelMoveVertices(DataModel *d, const vector &_param, const vector &_pos0);
	virtual ~ActionModelMoveVertices();
	string name(){	return "ModelMoveVertices";	}

	void *execute(Data *d);
	void undo(Data *d);
};

#endif /* ACTIONMODELMOVEVERTICES_H_ */
