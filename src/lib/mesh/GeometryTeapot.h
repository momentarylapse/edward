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

namespace GeometryTeapot {
	PolygonMesh create(const vec3 &pos, float radius, int samples);
};

#endif /* GEOMETRYTEAPOT_H_ */
