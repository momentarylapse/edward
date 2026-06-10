/*
 * Plane.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#pragma once

#include "../PolygonMesh.h"

struct ivec2;
struct rect;

namespace polymesh {
	PolygonMesh create_plane(const rect& r, const ivec2& slices);
};
