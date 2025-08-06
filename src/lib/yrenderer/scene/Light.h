//
// Created by michi on 8/3/25.
//
#pragma once

#include <lib/math/vec3.h>
#include <lib/math/mat4.h>
#include <lib/math/quaternion.h>
#include <lib/image/color.h>

namespace yrenderer {

struct CameraParams;


struct UBOLight {
	alignas(16) vec3 pos;
	float dummy;
	alignas(16) vec3 dir;
	alignas(16) color col;
	alignas(16) float radius;
	float theta, harshness;
	int shadow_index;
};

enum class LightType {
	DIRECTIONAL,
	POINT,
	CONE
};

struct Light {

	void init(const color &c, float r, float t);

	UBOLight to_ubo(const vec3& view_pos, const quaternion& view_ang, bool using_view_space) const;
	mat4 suggest_shadow_projection(const CameraParams& cam, float shadow_box_size) const;

	UBOLight light;
	quaternion _ang;
	bool enabled;
	bool allow_shadow;
	bool user_shadow_control;
	float user_shadow_theta;
	mat4 shadow_projection; // world -> texture
	float shadow_dist_min, shadow_dist_max;

	LightType type() const;
};

struct LightMetaData {
	int num_lights;
	int num_surfels;
	int dummy[2];
	ivec3 probe_cells;
	int _dummy3;
	vec3 probe_min;
	float _dummy4;
	vec3 probe_max;
	float _dummy5;
	mat4 shadow_proj[2];
};

} // yrenderer
