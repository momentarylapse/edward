//
// Created by michi on 8/3/25.
//

#include "Light.h"
#include "CameraParams.h"
#include <cmath>

namespace yrenderer {

void Light::init(LightType _type, const color &c, float t) {
	_ang = quaternion::ID;
	type = _type;
	pos = vec3(0,0,0);
	col = c;
	power = 1.0f;
	if (type == LightType::DIRECTIONAL) {
		theta = -1;
		harshness = 0.8f;
	} else {
		theta = t;
		harshness = 1;
	}
	enabled = true;
	allow_shadow = false;
	user_shadow_control = false;
	user_shadow_theta = -1;
	shadow_dist_min = -1;
	shadow_dist_max = -1;
}

float Light::radius() const {
	const float b = col.brightness() * power;
	return sqrtf(b) * 10.0f;
}

float Light::_radius_to_power(float radius) {
	return radius * radius / 100.0f;
}


UBOLight Light::to_ubo(const vec3& view_pos, const quaternion& view_ang, bool using_view_space) const {
	UBOLight l;
	l.col = col * power;
	l.harshness = harshness;
	if (type == LightType::DIRECTIONAL or type == LightType::AMBIENT)
		l.radius = -1;
	else
		l.radius = radius();
	l.theta = theta;
	if (using_view_space) {
		l.pos = view_ang.bar() * (pos - view_pos);
		l.dir = view_ang.bar() * _ang * vec3::EZ;
	} else {
		l.pos = pos;
		l.dir = _ang * vec3::EZ;
	}
	l.shadow_index = shadow_index;
	return l;
}

mat4 Light::suggest_shadow_projection(const CameraParams& cam, float shadow_box_size) const {
	if (type == LightType::DIRECTIONAL) {
		//msg_write(format("shadow dir: %s  %s", light.pos.str(), light.dir.str()));
		vec3 center = cam.pos + cam.ang*vec3::EZ * (shadow_box_size / 3.0f);
		float grid = shadow_box_size / 16;
		center.x -= fmodf(center.x, grid) - grid/2;
		center.y -= fmodf(center.y, grid) - grid/2;
		center.z -= fmodf(center.z, grid) - grid/2;
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
		auto t = mat4::translation(- pos);
		auto ang = cam.ang;
		if (type == LightType::CONE or user_shadow_control)
			ang = _ang;
		auto r = mat4::rotation(ang).transpose();
		float _theta = 1.35f;
		if (type == LightType::CONE)
			_theta = theta;
		if (user_shadow_theta > 0)
			_theta = user_shadow_theta;
		float _radius = radius();
		float dist_min = (shadow_dist_min > 0) ? shadow_dist_min : _radius * 0.01f;
		float dist_max = (shadow_dist_max > 0) ? shadow_dist_max : _radius;
		auto p = mat4::perspective(2 * _theta, 1.0f, dist_min, dist_max, false);
		return p * r * t;
	}
}
} // yrenderer