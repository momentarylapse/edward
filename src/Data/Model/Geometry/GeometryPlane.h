/*
 * GeometryPlane.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#ifndef GEOMETRYPLANE_H_
#define GEOMETRYPLANE_H_

#include "Geometry.h"

class GeometryPlane : public Geometry
{
public:
	GeometryPlane(const vector &pos, const vector &dv1, const vector &dv2, int num_x, int num_y);
	void _cdecl __init__(const vector &pos, const vector &dv1, const vector &dv2, int num_x, int num_y);
};

#endif /* GEOMETRYPLANE_H_ */
