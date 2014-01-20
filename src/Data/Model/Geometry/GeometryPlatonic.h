/*
 * GeometryPlatonic.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef GEOMETRYPLATONIC_H_
#define GEOMETRYPLATONIC_H_

#include "Geometry.h"
class vector;

class GeometryPlatonic : public Geometry
{
public:
	GeometryPlatonic(const vector &pos, float radius, int type);
	void _cdecl __init__(const vector &pos, float radius, int type);
	void AddTetrahedron(const vector &pos, float radius);
	void AddOctahedron(const vector &pos, float radius);
	void AddDodecahedron(const vector &pos, float radius);
	void AddIcosahedron(const vector &pos, float radius);
};

#endif /* GEOMETRYPLATONIC_H_ */
