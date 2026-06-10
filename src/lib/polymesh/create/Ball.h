/*
 * Ball.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#pragma once

#include "../PolygonMesh.h"

namespace polymesh {
	PolygonMesh create_ball(const vec3 &pos, float radius, int num_x, int num_y);
}



