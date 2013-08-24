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

template<class T>
class Interpolator;

class ModelGeometryCylinder : public ModelGeometry
{
public:
	ModelGeometryCylinder(Array<vector> &pos, Array<float> &radius, int rings, int edges, bool closed);
	ModelGeometryCylinder(Array<vector> &pos, float radius, int rings, int edges, bool closed);
	ModelGeometryCylinder(vector &pos1, vector &pos2, float radius, int rings, int edges, bool closed);
	_cdecl void __init2__(Array<vector> &pos, Array<float> &radius, int rings, int edges, bool closed);
	_cdecl void __init__(vector &pos1, vector &pos2, float radius, int rings, int edges, bool closed);

private:
	void BuildFromPath(Interpolator<vector> &inter, Interpolator<float> &inter_r, int rings, int edges, bool closed);
};

#endif /* MODELGEOMETRYCYLINDER_H_ */
