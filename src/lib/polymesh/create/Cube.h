/*
 * Cube.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#pragma once

#include "../PolygonMesh.h"

struct Box;

namespace polymesh {
	PolygonMesh create_cube(const Box& box, const ivec3& slices);
	PolygonMesh create_cube_x(const vec3 &_pos, const vec3 &dv1, const vec3 &dv2, const vec3 &dv3, int num_1, int num_2, int num_3);
}
