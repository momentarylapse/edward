/*
 * Light.cpp
 *
 *  Created on: 02.01.2020
 *      Author: michi
 */

#include "Light.h"
#include "Camera.h"
#include "../y/Entity.h"
//#include <lib/os/msg.h>

const kaba::Class *Light::_class = nullptr;

Light::Light(const color &c, float r, float t) {
	component_type = _class;
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
	type = LightType::DIRECTIONAL;
	if (light.radius > 0) {
		if (light.theta > 0)
			type = LightType::CONE;
		else
			type = LightType::POINT;
	}
}

void Light::on_init() {
	auto o = owner;
	light.pos = o->pos;
	light.dir = o->ang * vec3::EZ;
}

/*void Light::__init_parallel__(const quaternion &ang, const color &c) {
	new(this) Light(v_0, ang, c, -1, -1);
}
void Light::__init_spherical__(const vec3 &p, const color &c, float r) {
	new(this) Light(p, quaternion::ID, c, r, -1);
}
void Light::__init_cone__(const vec3 &p, const quaternion &ang, const color &c, float r, float t) {
	new(this) Light(p, ang, c, r, t);
}*/


void Light::set_direction(const vec3 &dir) {
	owner->ang = quaternion::rotation(dir.dir2ang());
}

void Light::update(Camera *cam, float shadow_box_size, bool using_view_space) {
	auto o = owner;
	if (using_view_space) {
		//light.pos = cam->m_view * o->pos;
		light.pos = cam->view_matrix() * o->pos;
		light.dir = cam->owner->ang.bar() * o->ang * vec3::EZ;
	} else {
		light.pos = o->pos;
		light.dir = o->ang * vec3::EZ;
	}

	if (allow_shadow) {
		if (type == LightType::DIRECTIONAL) {
			//msg_write(format("shadow dir: %s  %s", light.pos.str(), light.dir.str()));
			vec3 center = cam->owner->pos + cam->owner->ang*vec3::EZ * (shadow_box_size / 3.0f);
			float grid = shadow_box_size / 16;
			center.x -= fmod(center.x, grid) - grid/2;
			center.y -= fmod(center.y, grid) - grid/2;
			center.z -= fmod(center.z, grid) - grid/2;
			//center = vec3(0,200,0);
			auto t = mat4::translation(- center);
			//auto r = mat4::rotation({pi/2,0,0}).transpose();
			//o->ang = quaternion(pi/2, {1,0,0});
			auto r = mat4::rotation(o->ang).transpose();
			float f = 1 / shadow_box_size;
			auto s = mat4::scale(f, f, f);
			// map onto [-1,1]x[-1,1]x[0,1]
			shadow_projection = mat4::translation(vec3(0,0,0.5f)) * mat4::scale(1,1,0.5f) * mat4::translation(vec3(0,0,-0.5f)) * s * r * t;
			//msg_write(shadow_projection.str());
		} else {
			auto t = mat4::translation(- o->pos);
			auto ang = cam->owner->ang;
			if (type == LightType::CONE or user_shadow_control)
				ang = o->ang;
			auto r = mat4::rotation(ang).transpose();
			float theta = 1.35f;
			if (type == LightType::CONE)
				theta = light.theta;
			if (user_shadow_theta > 0)
				theta = user_shadow_theta;
			float dist_min = (shadow_dist_min > 0) ? shadow_dist_min : light.radius * 0.01f;
			float dist_max = (shadow_dist_max > 0) ? shadow_dist_max : light.radius;
			auto p = mat4::perspective(2 * theta, 1.0f, dist_min, dist_max, false);
			shadow_projection = p * r * t;
		}
		if (using_view_space)
			light.proj = shadow_projection * cam->view_matrix().inverse();
		else
			light.proj = shadow_projection;
	}
}
