/*
 * ActionModelExtrudePolygons.h
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELEXTRUDEPOLYGONS_H_
#define ACTIONMODELEXTRUDEPOLYGONS_H_

#include "../../../ActionGroup.h"
class DataModel;
class ModelSurface;
class vector;

class ActionModelExtrudePolygons: public ActionGroup
{
public:
	ActionModelExtrudePolygons(float _offset);
	virtual ~ActionModelExtrudePolygons(){}
	string name(){	return "ModelExtrudePolygons";	}

	void *compose(Data *d);

private:
	void ExtrudeSurface(ModelSurface &s, int surface, DataModel *m);
	float offset;
};

#endif /* ACTIONMODELEXTRUDEPOLYGONS_H_ */
