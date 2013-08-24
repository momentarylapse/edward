/*
 * ModelGeometryPlane.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#ifndef MODELGEOMETRYPLANE_H_
#define MODELGEOMETRYPLANE_H_

#include "ModelGeometry.h"

class ModelGeometryPlane : public ModelGeometry
{
public:
	ModelGeometryPlane(const vector &pos, const vector &dv1, const vector &dv2, int num_x, int num_y);
	_cdecl void __init__(const vector &pos, const vector &dv1, const vector &dv2, int num_x, int num_y);
};

#endif /* MODELGEOMETRYPLANE_H_ */
