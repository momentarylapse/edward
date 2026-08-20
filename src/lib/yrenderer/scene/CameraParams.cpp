//
// Created by michi on 8/2/25.
//

#include "CameraParams.h"
#include <cmath>

namespace yrenderer {

mat4 CameraParams::projection_matrix(float aspect_ratio) const {
	// flip the y-axis
	float a = tanf(fov / 2);
	return mat4::translation({offset.x/(a*aspect_ratio), offset.y/a, 0}) * mat4::perspective(fov, aspect_ratio, min_depth, max_depth, false) * mat4::scale(1,-1,1);
}

mat4 CameraParams::view_matrix() const {
	return mat4::rotation(ang).transpose() * mat4::translation(-pos);
}

} // yrenderer