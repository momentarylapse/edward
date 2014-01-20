/*
 * GeometryTorusKnot.h
 *
 *  Created on: 26.05.2013
 *      Author: michi
 */

#ifndef GEOMETRYTORUSKNOT_H_
#define GEOMETRYTORUSKNOT_H_

#include "Geometry.h"

class GeometryTorusKnot : public Geometry
{
public:
	GeometryTorusKnot(const vector &pos, const vector &axis, float radius1, float radius2, float radius3, int turns, int twists, int rings, int edges);
	void _cdecl __init__(const vector &pos, const vector &axis, float radius1, float radius2, float radius3, int turns, int twists, int rings, int edges);
};

#endif /* GEOMETRYTORUSKNOT_H_ */
