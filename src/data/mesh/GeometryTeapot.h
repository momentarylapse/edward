/*
 * GeometryTeapot.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef GEOMETRYTEAPOT_H_
#define GEOMETRYTEAPOT_H_

#include "PolygonMesh.h"
struct vec3;

class GeometryTeapot : public PolygonMesh {
public:
	GeometryTeapot(const vec3 &pos, float radius, int samples);
	static PolygonMesh create(const vec3 &pos, float radius, int samples) {
		return GeometryTeapot(pos, radius, samples);
	}
};

#endif /* GEOMETRYTEAPOT_H_ */
