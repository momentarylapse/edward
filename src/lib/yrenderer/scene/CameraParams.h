//
// Created by michi on 8/2/25.
//

#pragma once

#include <lib/math/vec3.h>
#include <lib/math/quaternion.h>
#include <lib/math/mat4.h>

namespace yrenderer {

struct CameraParams {
	vec3 pos = vec3::ZERO;
	quaternion ang = quaternion::ID;
	float fov = 1.0f;
	float min_depth = 0.1f, max_depth = 1000.0f;


	// view space -> relative screen space (API independent)
	// (-1,-1) = top left
	// (+1,+1) = bottom right
	mat4 projection_matrix(float aspect_ratio) const;
	mat4 view_matrix() const;
};

} // yrenderer
