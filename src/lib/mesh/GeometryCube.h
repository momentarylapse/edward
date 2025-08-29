/*
 * GeometryCube.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#ifndef GEOMETRYCUBE_H_
#define GEOMETRYCUBE_H_

#include "PolygonMesh.h"

struct Box;

class GeometryCube : public PolygonMesh {
public:
	GeometryCube(const vec3 &pos, const vec3 &dv1, const vec3 &dv2, const vec3 &dv3, int num_1, int num_2, int num_3);

	static PolygonMesh create(const Box& box, const ivec3& slices);
};

#endif /* GEOMETRYCUBE_H_ */
