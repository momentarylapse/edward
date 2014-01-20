/*
 * GeometryCylinder.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef GEOMETRYCYLINDER_H_
#define GEOMETRYCYLINDER_H_

#include "Geometry.h"
class vector;

template<class T>
class Interpolator;

class GeometryCylinder : public Geometry
{
public:
	GeometryCylinder(Array<vector> &pos, Array<float> &radius, int rings, int edges, bool closed);
	GeometryCylinder(Array<vector> &pos, float radius, int rings, int edges, bool closed);
	GeometryCylinder(const vector &pos1, const vector &pos2, float radius, int rings, int edges, bool closed);
	void _cdecl __init2__(Array<vector> &pos, Array<float> &radius, int rings, int edges, bool closed);
	void _cdecl __init__(const vector &pos1, const vector &pos2, float radius, int rings, int edges, bool closed);

private:
	void BuildFromPath(Interpolator<vector> &inter, Interpolator<float> &inter_r, int rings, int edges, bool closed);
};

#endif /* GEOMETRYCYLINDER_H_ */
