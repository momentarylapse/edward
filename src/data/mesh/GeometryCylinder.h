/*
 * GeometryCylinder.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef GEOMETRYCYLINDER_H_
#define GEOMETRYCYLINDER_H_

#include "PolygonMesh.h"
struct vec3;

template<class T>
class Interpolator;

class GeometryCylinder : public PolygonMesh {
public:
	GeometryCylinder(Array<vec3> &pos, Array<float> &radius, int rings, int edges, int end_mode = END_FLAT);
	GeometryCylinder(Array<vec3> &pos, float radius, int rings, int edges, int end_mode = END_FLAT);
	GeometryCylinder(const vec3 &pos1, const vec3 &pos2, float radius, int rings, int edges, int end_mode = END_FLAT);
	void _cdecl __init2__(Array<vec3> &pos, Array<float> &radius, int rings, int edges, int end_mode);
	void _cdecl __init__(const vec3 &pos1, const vec3 &pos2, float radius, int rings, int edges, int end_mode);

	enum{
		END_OPEN,
		END_FLAT,
		END_ROUND,
		END_LOOP
	};

private:
	void buildFromPath(Interpolator<vec3> &inter, Interpolator<float> &inter_r, int rings, int edges, int end_mode);
};

#endif /* GEOMETRYCYLINDER_H_ */
