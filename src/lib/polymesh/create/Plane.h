/*
 * Plane.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#pragma once

#include "../Mesh.h"

struct ivec2;
struct rect;

namespace polymesh {
	Mesh create_plane(const rect& r, const ivec2& slices);
};
