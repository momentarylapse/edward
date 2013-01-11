/*
 * ModelGeometryTorus.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef MODELGEOMETRYTORUS_H_
#define MODELGEOMETRYTORUS_H_

#include "ModelGeometry.h"
class vector;

class ModelGeometryTorus : public ModelGeometry
{
public:
	ModelGeometryTorus(const vector &pos, const vector &axis, float radius1, float radius2, int num_x, int num_y);
};

#endif /* MODELGEOMETRYTORUS_H_ */
