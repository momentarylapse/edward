/*
 * ActionModelPolygonAddVertex.h
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELPOLYGONADDVERTEX_H_
#define ACTIONMODELPOLYGONADDVERTEX_H_

#include "../../../../ActionGroup.h"
#include "../../../../../lib/x/x.h"

class ActionModelPolygonAddVertex: public ActionGroup
{
public:
	ActionModelPolygonAddVertex(int _surface, int _poly, int _side, int _vertex, const vector *_sv);
	virtual ~ActionModelPolygonAddVertex(){}
	string name(){	return "ModelPolygonAddVertex";	}

	void *compose(Data *d);

private:
	int surface;
	int poly;
	int side;
	int vertex;
	vector sv[MODEL_MAX_TEXTURES];
};

#endif /* ACTIONMODELPOLYGONADDVERTEX_H_ */
