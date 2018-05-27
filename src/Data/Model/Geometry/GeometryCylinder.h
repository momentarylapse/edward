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
	GeometryCylinder(Array<vector> &pos, Array<float> &radius, int rings, int edges, int end_mode = END_FLAT);
	GeometryCylinder(Array<vector> &pos, float radius, int rings, int edges, int end_mode = END_FLAT);
	GeometryCylinder(const vector &pos1, const vector &pos2, float radius, int rings, int edges, int end_mode = END_FLAT);
	void _cdecl __init2__(Array<vector> &pos, Array<float> &radius, int rings, int edges, int end_mode);
	void _cdecl __init__(const vector &pos1, const vector &pos2, float radius, int rings, int edges, int end_mode);

	enum{
		END_OPEN,
		END_FLAT,
		END_ROUND,
		END_LOOP
	};

private:
	void buildFromPath(Interpolator<vector> &inter, Interpolator<float> &inter_r, int rings, int edges, int end_mode);
};

#endif /* GEOMETRYCYLINDER_H_ */
