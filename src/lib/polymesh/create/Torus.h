/*
 * Torus.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#pragma once

#include "../Mesh.h"
struct vec3;


namespace polymesh {
	Mesh create_torus(const vec3 &pos, const vec3 &axis, float radius1, float radius2, int num_x, int num_y);
}


