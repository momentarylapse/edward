/*
 * ActionModelScaleVertices.h
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSCALEVERTICES_H_
#define ACTIONMODELSCALEVERTICES_H_

#include "../../../ActionMultiView.h"
class vector;
class DataModel;

class ActionModelScaleVertices: public ActionMultiView
{
public:
	ActionModelScaleVertices(DataModel *d, const vector &_param, const vector &_pos0, const vector &_ex, const vector &_ey, const vector &_ez);
	virtual ~ActionModelScaleVertices();
	string name(){	return "ModelScaleVertices";	}

	void *execute(Data *d);
	void undo(Data *d);
private:
	vector e[3];
};

#endif /* ACTIONMODELSCALEVERTICES_H_ */
