/*
 * ActionModelAddCylinder.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDCYLINDER_H_
#define ACTIONMODELADDCYLINDER_H_

#include "../../../ActionGroup.h"
class DataModel;
class Data;
class vector;

class ActionModelAddCylinder: public ActionGroup
{
public:
	ActionModelAddCylinder(DataModel *m, Array<vector> &pos, Array<float> &radius, int rings, int edges, bool closed);
	virtual ~ActionModelAddCylinder();
	string name(){	return "ModelAddCylinder";	}

	void *compose(Data *d);
};

#endif /* ACTIONMODELADDCYLINDER_H_ */
