//
// Created by michi on 8/3/25.
//

#include "Light.h"
#include "CameraParams.h"

namespace yrenderer {

void Light::init(const color &c, float r, float t) {
	_ang = quaternion::ID;
	light.pos = vec3(0,0,0);
	light.dir = vec3::EZ;
	light.col = c;
	light.radius = r;
	light.theta = t;
	light.harshness = 0.8f;
	if (light.radius >= 0)
		light.harshness = 1;
	enabled = true;
	allow_shadow = false;
	user_shadow_control = false;
	user_shadow_theta = -1;
	shadow_dist_min = -1;
	shadow_dist_max = -1;
}

LightType Light::type() const {
	if (light.radius <= 0)
		return LightType::DIRECTIONAL;
	if (light.theta > 0)
		return LightType::CONE;
	return LightType::POINT;
}


UBOLight Light::to_ubo(const vec3& view_pos, const quaternion& view_ang, bool using_view_space) const {
	UBOLight l;
	l.col = light.col;
	l.harshness = light.harshness;
	l.radius = light.radius;
	l.theta = light.theta;
	if (using_view_space) {
		l.pos = view_ang.bar() * (light.pos - view_pos);
		l.dir = view_ang.bar() * _ang * vec3::EZ;
	} else {
		l.pos = light.pos;
		l.dir = _ang * vec3::EZ;
	}
	l.shadow_index = light.shadow_index;
	return l;
}

mat4 Light::suggest_shadow_projection(const CameraParams& cam, float shadow_box_size) const {
	if (type() == LightType::DIRECTIONAL) {
		//msg_write(format("shadow dir: %s  %s", light.pos.str(), light.dir.str()));
		vec3 center = cam.pos + cam.ang*vec3::EZ * (shadow_box_size / 3.0f);
		float grid = shadow_box_size / 16;
		center.x -= fmod(center.x, grid) - grid/2;
		center.y -= fmod(center.y, grid) - grid/2;
		center.z -= fmod(center.z, grid) - grid/2;
		//center = vec3(0,200,0);
		auto t = mat4::translation(- center);
		//auto r = mat4::rotation({pi/2,0,0}).transpose();
		//o->ang = quaternion(pi/2, {1,0,0});
		auto r = mat4::rotation(_ang).transpose();
		float f = 1 / shadow_box_size;
		auto s = mat4::scale(f, f, f);
		// map onto [-1,1]x[-1,1]x[0,1]
		return mat4::translation(vec3(0,0,0.5f)) * mat4::scale(1,1,0.5f) * mat4::translation(vec3(0,0,-0.5f)) * s * r * t;
		//msg_write(shadow_projection.str());
	} else {
		auto t = mat4::translation(- light.pos);
		auto ang = cam.ang;
		if (type() == LightType::CONE or user_shadow_control)
			ang = _ang;
		auto r = mat4::rotation(ang).transpose();
		float theta = 1.35f;
		if (type() == yrenderer::LightType::CONE)
			theta = light.theta;
		if (user_shadow_theta > 0)
			theta = user_shadow_theta;
		float dist_min = (shadow_dist_min > 0) ? shadow_dist_min : light.radius * 0.01f;
		float dist_max = (shadow_dist_max > 0) ? shadow_dist_max : light.radius;
		auto p = mat4::perspective(2 * theta, 1.0f, dist_min, dist_max, false);
		return p * r * t;
	}
}
} // yrenderer