/*
 * ModelGeometryCylinder.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef MODELGEOMETRYCYLINDER_H_
#define MODELGEOMETRYCYLINDER_H_

#include "ModelGeometry.h"
class vector;

class ModelGeometryCylinder : public ModelGeometry
{
public:
	ModelGeometryCylinder(Array<vector> &pos, Array<float> &radius, int rings, int edges, bool closed);
};

#endif /* MODELGEOMETRYCYLINDER_H_ */
