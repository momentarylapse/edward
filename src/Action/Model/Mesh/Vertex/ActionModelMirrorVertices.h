/*
 * ActionModelMirrorVertices.h
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMIRRORVERTICES_H_
#define ACTIONMODELMIRRORVERTICES_H_

#include "../../../ActionMultiView.h"
class vector;
class DataModel;

class ActionModelMirrorVertices: public ActionMultiView
{
public:
	ActionModelMirrorVertices(DataModel *d, const vector &_param, const vector &_pos0);
	virtual ~ActionModelMirrorVertices();
	string name(){	return "ModelMirrorVertices";	}

	void *execute(Data *d);
	void undo(Data *d);
};

#endif /* ACTIONMODELMIRRORVERTICES_H_ */
