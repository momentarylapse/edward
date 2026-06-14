/*
 * Teapot.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#pragma once

#include "../Mesh.h"
struct vec3;

namespace polymesh {
	Mesh create_teapot(const vec3 &pos, float radius, int samples);
}

