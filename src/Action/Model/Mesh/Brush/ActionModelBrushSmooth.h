/*
 * ActionModelBrushSmooth.h
 *
 *  Created on: 31.08.2013
 *      Author: michi
 */

#ifndef ACTIONMODELBRUSHSMOOTH_H_
#define ACTIONMODELBRUSHSMOOTH_H_

#include "../../../../lib/math/vec3.h"
#include "../../../Action.h"
class DataModel;

class ActionModelBrushSmooth: public Action {
public:
	ActionModelBrushSmooth(const vec3 &pos, const vec3 &n, float radius);
	string name(){	return "ModelBrushSmooth";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	vec3 pos, n;
	float radius;
	Array<int> index;
	Array<vec3> pos_old;
};

#endif /* ACTIONMODELBRUSHSMOOTH_H_ */
