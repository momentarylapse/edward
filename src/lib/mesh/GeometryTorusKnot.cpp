/*
 * GeometryTorusKnot.cpp
 *
 *  Created on: 26.05.2013
 *      Author: michi
 */

#include "GeometryTorusKnot.h"
#include "GeometryCylinder.h"
#include <cmath>

GeometryTorusKnot::GeometryTorusKnot(const vec3 &_pos, const vec3 &axis, float radius1, float radius2, float radius3, int turns, int twists, int rings, int edges) {
	Array<vec3> pos;
	for (int i=0; i<rings;i++) {
		float t = (float)i / rings;
		float wx = 2 * pi * t * turns;
		float wy = 2 * pi * t * twists;

		vec3 er = vec3(cos(wx), sin(wx), 0);
		pos.add(er * radius1 + (er * cos(wy) + vec3::EZ * sin(wy)) * radius2);
	}
	GeometryCylinder geo = GeometryCylinder(pos, radius3, rings, edges, GeometryCylinder::END_LOOP);
	add(geo);
}
