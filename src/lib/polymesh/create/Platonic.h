/*
 * Platonic.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#pragma once

#include "../Mesh.h"
struct vec3;

namespace polymesh {
	Mesh create_tetrahedron(const vec3& pos, float radius);
	Mesh create_octahedron(const vec3& pos, float radius);
	Mesh create_dodecahedron(const vec3& pos, float radius);
	Mesh create_icosahedron(const vec3& pos, float radius);

	Mesh create_platonic(const vec3& pos, float radius, int type);
}

