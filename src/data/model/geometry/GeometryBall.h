/*
 * GeometryBall.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#ifndef GEOMETRYBALL_H_
#define GEOMETRYBALL_H_

#include "Geometry.h"

class GeometryBall : public Geometry
{
public:
	GeometryBall(const vec3 &pos, float radius, int num_x, int num_y);
	void _cdecl __init__(const vec3 &pos, float radius, int num_x, int num_y);
};

#endif /* GEOMETRYBALL_H_ */
