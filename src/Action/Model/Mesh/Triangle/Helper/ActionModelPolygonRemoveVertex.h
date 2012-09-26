/*
 * ActionModelPolygonRemoveVertex.h
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELPOLYGONREMOVEVERTEX_H_
#define ACTIONMODELPOLYGONREMOVEVERTEX_H_

#include "../../../../ActionGroup.h"

class ActionModelPolygonRemoveVertex: public ActionGroup
{
public:
	ActionModelPolygonRemoveVertex(int _surface, int _poly, int _side);
	virtual ~ActionModelPolygonRemoveVertex(){}
	string name(){	return "ModelPolygonRemoveVertex";	}

	void *compose(Data *d);

private:
	int surface;
	int poly;
	int side;
};

#endif /* ACTIONMODELPOLYGONREMOVEVERTEX_H_ */
