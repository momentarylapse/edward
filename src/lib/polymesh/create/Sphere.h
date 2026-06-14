/*
 * Sphere.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#pragma once

#include "../Mesh.h"
struct vec3;

namespace polymesh {
	Mesh create_sphere(const vec3& pos, float radius, int num);
}
