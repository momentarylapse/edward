/*
 * Cylinder.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#pragma once

#include "../PolygonMesh.h"
struct vec3;


namespace polymesh {
	enum class End {
		OPEN,
		FLAT,
		ROUND,
		LOOP
	};
	PolygonMesh create_cylinder(Array<vec3> &pos, Array<float> &radius, int rings, int edges, End end_mode = End::FLAT);
	PolygonMesh create_cylinder(Array<vec3> &pos, float radius, int rings, int edges, End end_mode = End::FLAT);
	PolygonMesh create_cylinder(const vec3 &pos1, const vec3 &pos2, float radius, int rings, int edges, End end_mode = End::FLAT);
}

