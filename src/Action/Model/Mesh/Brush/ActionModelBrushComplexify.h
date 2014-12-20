/*
 * ActionModelBrushComplexify.h
 *
 *  Created on: 20.12.2014
 *      Author: michi
 */

#ifndef SRC_ACTION_MODEL_MESH_BRUSH_ACTIONMODELBRUSHCOMPLEXIFY_H_
#define SRC_ACTION_MODEL_MESH_BRUSH_ACTIONMODELBRUSHCOMPLEXIFY_H_

#include "../../../ActionGroup.h"
#include "../../../../lib/math/math.h"
class DataModel;

class ActionModelBrushComplexify: public ActionGroup
{
public:
	ActionModelBrushComplexify(const vector &pos, const vector &n, float radius, float min_dist);
	string name(){	return "ModelBrushComplexify";	}

	void *compose(Data *d);

	vector pos;
	vector n;
	float radius;
	float min_dist;
};

#endif /* SRC_ACTION_MODEL_MESH_BRUSH_ACTIONMODELBRUSHCOMPLEXIFY_H_ */
