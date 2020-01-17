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

class ActionModelExtrudePolygons: public ActionGroup {
public:
	ActionModelExtrudePolygons(float offset, bool independent);
	string name() { return "ModelExtrudePolygons"; }

	void *compose(Data *d);

private:
	void extrude_surface(ModelSurface &s, int surface, DataModel *m);
	void extrude_surface_indep(ModelSurface &s, int surface, DataModel *m);
	float offset;
	bool independent;
};

#endif /* ACTIONMODELEXTRUDEPOLYGONS_H_ */
