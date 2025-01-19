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
class vec3;

class ActionModelExtrudePolygons: public ActionGroup {
public:
	ActionModelExtrudePolygons(float offset, bool independent, int material);
	string name() { return "ModelExtrudePolygons"; }

	void *compose(Data *d);

private:
	void extrude_surface(DataModel *m);
	void extrude_surface_indep(DataModel *m);
	float offset;
	bool independent;
	int material;
};

#endif /* ACTIONMODELEXTRUDEPOLYGONS_H_ */
