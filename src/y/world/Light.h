/*
 * Light.h
 *
 *  Created on: 02.01.2020
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>
#include <lib/math/vec3.h>
#include <lib/math/mat4.h>
#include <lib/image/color.h>
#include "../y/Component.h"

class Camera;

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

class Light : public Component {
public:
	Light(const color &c, float r, float t);
	/*void __init_parallel__(const quaternion &ang, const color &c);
	void __init_spherical__(const vec3 &p, const color &c, float r);
	void __init_cone__(const vec3 &p, const quaternion &ang, const color &c, float r, float t);*/

	void on_init() override;

	void set_direction(const vec3 &dir);

	UBOLight to_ubo(const vec3& view_pos, const quaternion& view_ang, bool using_view_space) const;
	mat4 suggest_shadow_projection(Camera *cam, float shadow_box_size) const;

	UBOLight light;
	bool enabled;
	bool allow_shadow;
	bool user_shadow_control;
	float user_shadow_theta;
	mat4 shadow_projection; // world -> texture
	float shadow_dist_min, shadow_dist_max;

	LightType type() const;

	static const kaba::Class *_class;
};


