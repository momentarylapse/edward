/*
 * GeometryBall.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#ifndef GEOMETRYBALL_H_
#define GEOMETRYBALL_H_

#include "PolygonMesh.h"

class GeometryBall : public PolygonMesh {
public:
	GeometryBall(const vec3 &pos, float radius, int num_x, int num_y);
	static PolygonMesh create(const vec3 &pos, float radius, int num_x, int num_y) {
		return GeometryBall(pos, radius, num_x, num_y);
	}
};



#endif /* GEOMETRYBALL_H_ */
