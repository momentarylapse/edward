/*
 * GeometryPlane.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#ifndef GEOMETRYPLANE_H_
#define GEOMETRYPLANE_H_

#include "PolygonMesh.h"

class GeometryPlane : public PolygonMesh {
public:
	GeometryPlane(const vec3 &pos, const vec3 &dv1, const vec3 &dv2, int num_x, int num_y);
	static PolygonMesh create(const vec3 &pos, const vec3 &dv1, const vec3 &dv2, int num_x, int num_y) {
		return GeometryPlane(pos, dv1, dv2, num_x, num_y);
	}
};

#endif /* GEOMETRYPLANE_H_ */
