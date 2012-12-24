/*
 * ActionModelAddCylinder.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDCYLINDER_H_
#define ACTIONMODELADDCYLINDER_H_

#include "../../../ActionGroup.h"
#include "../../../../lib/types/vector.h"

class ActionModelAddCylinder: public ActionGroup
{
public:
	ActionModelAddCylinder(Array<vector> &pos, Array<float> &radius, int rings, int edges, bool closed);
	virtual ~ActionModelAddCylinder();
	string name(){	return "ModelAddCylinder";	}

	void *compose(Data *d);
private:
	Array<vector> pos;
	Array<float> radius;
	int rings, edges;
	bool closed;
};

#endif /* ACTIONMODELADDCYLINDER_H_ */
