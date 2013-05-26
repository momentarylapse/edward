/*
 * ModelGeometryTorusKnot.h
 *
 *  Created on: 26.05.2013
 *      Author: michi
 */

#ifndef MODELGEOMETRYTORUSKNOT_H_
#define MODELGEOMETRYTORUSKNOT_H_

#include "ModelGeometry.h"

class ModelGeometryTorusKnot : public ModelGeometry
{
public:
	ModelGeometryTorusKnot(const vector &pos, const vector &axis, float radius1, float radius2, float radius3, int turns, int twists, int rings, int edges);
};

#endif /* MODELGEOMETRYTORUSKNOT_H_ */
