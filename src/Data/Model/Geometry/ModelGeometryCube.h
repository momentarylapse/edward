/*
 * ModelGeometryCube.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#ifndef MODELGEOMETRYCUBE_H_
#define MODELGEOMETRYCUBE_H_

#include "ModelGeometry.h"

class ModelGeometryCube : public ModelGeometry
{
public:
	ModelGeometryCube(const vector &pos, const vector &dv1, const vector &dv2, const vector &dv3, int num_1, int num_2, int num_3);
	_cdecl void __init__(const vector &pos, const vector &dv1, const vector &dv2, const vector &dv3, int num_1, int num_2, int num_3);
};

#endif /* MODELGEOMETRYCUBE_H_ */
