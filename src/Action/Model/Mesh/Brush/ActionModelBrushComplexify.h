/*
 * ActionModelBrushComplexify.h
 *
 *  Created on: 20.12.2014
 *      Author: michi
 */

#ifndef SRC_ACTION_MODEL_MESH_BRUSH_ACTIONMODELBRUSHCOMPLEXIFY_H_
#define SRC_ACTION_MODEL_MESH_BRUSH_ACTIONMODELBRUSHCOMPLEXIFY_H_

#include "../../../../lib/math/vec3.h"
#include "../../../ActionGroup.h"
class DataModel;

class ActionModelBrushComplexify: public ActionGroup {
public:
	ActionModelBrushComplexify(const vec3 &pos, const vec3 &n, float radius, float min_dist);
	string name(){	return "ModelBrushComplexify";	}

	void *compose(Data *d);

	vec3 pos;
	vec3 n;
	float radius;
	float min_dist;
};

#endif /* SRC_ACTION_MODEL_MESH_BRUSH_ACTIONMODELBRUSHCOMPLEXIFY_H_ */
