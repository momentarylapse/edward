/*
 * ActionModelCutOutPolygons.h
 *
 *  Created on: 28.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELCUTOUTPOLYGONS_H_
#define ACTIONMODELCUTOUTPOLYGONS_H_

#include "../../../ActionGroup.h"
class DataModel;
class ModelSurface;
class vector;

class ActionModelCutOutPolygons: public ActionGroup
{
public:
	ActionModelCutOutPolygons();
	virtual ~ActionModelCutOutPolygons(){}
	string name(){	return "ModelCutOutPolygons";	}

	void *compose(Data *d);

private:
	void CutOutSurface(ModelSurface &s, int surface, DataModel *m);
};

#endif /* ACTIONMODELCUTOUTPOLYGONS_H_ */