/*
 * ModelGeometryBall.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#ifndef MODELGEOMETRYBALL_H_
#define MODELGEOMETRYBALL_H_

#include "ModelGeometry.h"

class ModelGeometryBall : public ModelGeometry
{
public:
	ModelGeometryBall(const vector &pos, float radius, int num_x, int num_y);
	_cdecl void __init__(const vector &pos, float radius, int num_x, int num_y);
};

#endif /* MODELGEOMETRYBALL_H_ */
