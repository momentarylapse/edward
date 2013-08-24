/*
 * ModelGeometryTeapot.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef MODELGEOMETRYTEAPOT_H_
#define MODELGEOMETRYTEAPOT_H_

#include "ModelGeometry.h"
class vector;

class ModelGeometryTeapot : public ModelGeometry
{
public:
	ModelGeometryTeapot(const vector &pos, float radius, int samples);
	_cdecl void __init__(const vector &pos, float radius, int samples);
};

#endif /* MODELGEOMETRYTEAPOT_H_ */
