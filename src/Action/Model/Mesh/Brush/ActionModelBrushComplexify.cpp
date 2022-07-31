/*
 * ActionModelBrushComplexify.cpp
 *
 *  Created on: 20.12.2014
 *      Author: michi
 */

#include "ActionModelBrushComplexify.h"

ActionModelBrushComplexify::ActionModelBrushComplexify(const vec3 &_pos, const vec3 &_n, float _radius, float _min_dist) {
	pos = _pos;
	n = _n;
	radius = _radius;
	min_dist = _min_dist;
}

void *ActionModelBrushComplexify::compose(Data *d) {
	// TODO: make list of all edges in selection < min_dist
	// split edges
	// all polygons in selection: if radius too large: add center
	return nullptr;
}

