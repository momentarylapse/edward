/*
 * ActionAddPolygon.h
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDPOLYGON_H_
#define ACTIONMODELADDPOLYGON_H_

#include "../../../ActionGroup.h"
class DataModel;
class vec3;

class ActionModelAddPolygon: public ActionGroup {
public:
	ActionModelAddPolygon(Array<int> &_v, int _material, Array<vec3> &_sv);
	string name() { return "ModelAddPolygon"; }

	void *compose(Data *d);

private:
	Array<int> &v;
	int material;
	Array<vec3> &sv;
};

#endif /* ACTIONMODELADDPOLYGON_H_ */
