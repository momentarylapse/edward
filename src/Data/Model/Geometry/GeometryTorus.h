/*
 * GeometryTorus.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef GEOMETRYTORUS_H_
#define GEOMETRYTORUS_H_

#include "Geometry.h"
class vector;

class GeometryTorus : public Geometry
{
public:
	GeometryTorus(const vector &pos, const vector &axis, float radius1, float radius2, int num_x, int num_y);
	void _cdecl __init__(const vector &pos, const vector &axis, float radius1, float radius2, int num_x, int num_y);
};

#endif /* GEOMETRYTORUS_H_ */
