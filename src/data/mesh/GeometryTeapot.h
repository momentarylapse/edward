/*
 * GeometryTeapot.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef GEOMETRYTEAPOT_H_
#define GEOMETRYTEAPOT_H_

#include "PolygonMesh.h"
class vec3;

class GeometryTeapot : public PolygonMesh
{
public:
	GeometryTeapot(const vec3 &pos, float radius, int samples);
	void _cdecl __init__(const vec3 &pos, float radius, int samples);
};

#endif /* GEOMETRYTEAPOT_H_ */
