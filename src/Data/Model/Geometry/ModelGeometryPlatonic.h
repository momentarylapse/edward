/*
 * ModelGeometryPlatonic.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef MODELGEOMETRYPLATONIC_H_
#define MODELGEOMETRYPLATONIC_H_

#include "ModelGeometry.h"
class vector;

class ModelGeometryPlatonic : public ModelGeometry
{
public:
	ModelGeometryPlatonic(const vector &pos, float radius, int type);
	_cdecl void __init__(const vector &pos, float radius, int type);
	void AddTetrahedron(const vector &pos, float radius);
	void AddOctahedron(const vector &pos, float radius);
	void AddDodecahedron(const vector &pos, float radius);
	void AddIcosahedron(const vector &pos, float radius);
};

#endif /* MODELGEOMETRYPLATONIC_H_ */
