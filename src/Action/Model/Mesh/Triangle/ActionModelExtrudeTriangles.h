/*
 * ActionModelExtrudeTriangles.h
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELEXTRUDETRIANGLES_H_
#define ACTIONMODELEXTRUDETRIANGLES_H_

#include "../../../ActionGroup.h"
class DataModel;
class ModelSurface;
class vector;

class ActionModelExtrudeTriangles: public ActionGroup
{
public:
	ActionModelExtrudeTriangles(DataModel *data, float offset);
	virtual ~ActionModelExtrudeTriangles();
	string name(){	return "ModelExtrudeTriangles";	}

private:
	void ExtrudeSurface(ModelSurface &s, int surface, DataModel *m, const vector &dpos);
};

#endif /* ACTIONMODELEXTRUDETRIANGLES_H_ */
