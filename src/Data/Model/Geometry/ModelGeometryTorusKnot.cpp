/*
 * ModelGeometryTorusKnot.cpp
 *
 *  Created on: 26.05.2013
 *      Author: michi
 */

#include "ModelGeometryTorusKnot.h"
#include "ModelGeometryCylinder.h"

ModelGeometryTorusKnot::ModelGeometryTorusKnot(const vector &_pos, const vector &axis, float radius1, float radius2, float radius3, int turns, int twists, int rings, int edges)
{
	Array<vector> pos;
	for (int i=0; i<rings;i++){
		float t = (float)i / rings;
		float wx = 2 * pi * t * turns;
		float wy = 2 * pi * t * twists;

		vector er = vector(cos(wx), sin(wx), 0);
		pos.add(er * radius1 + (er * cos(wy) + e_z * sin(wy)) * radius2);
	}
	ModelGeometryCylinder geo = ModelGeometryCylinder(pos, radius3, rings, edges, true);
	Add(geo);
}

void ModelGeometryTorusKnot::__init__(const vector& pos, const vector& axis, float radius1, float radius2, float radius3, int turns, int twists, int rings, int edges)
{
	new (this) ModelGeometryTorusKnot(pos, axis, radius1, radius2, radius3, turns, twists, rings, edges);
}
