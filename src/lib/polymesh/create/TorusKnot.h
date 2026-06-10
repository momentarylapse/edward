/*
 * TorusKnot.h
 *
 *  Created on: 26.05.2013
 *      Author: michi
 */

#pragma once

#include "../PolygonMesh.h"

namespace polymesh {
	PolygonMesh create_torus_knot(const vec3 &pos, const vec3 &axis, float radius1, float radius2, float radius3, int turns, int twists, int rings, int edges);
}

