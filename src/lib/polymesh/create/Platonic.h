/*
 * Platonic.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#pragma once

#include "../PolygonMesh.h"
struct vec3;

namespace polymesh {
	PolygonMesh create_tetrahedron(const vec3& pos, float radius);
	PolygonMesh create_octahedron(const vec3& pos, float radius);
	PolygonMesh create_dodecahedron(const vec3& pos, float radius);
	PolygonMesh create_icosahedron(const vec3& pos, float radius);

	PolygonMesh create_platonic(const vec3& pos, float radius, int type);
}

