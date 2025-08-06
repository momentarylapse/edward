/*
 * Light.cpp
 *
 *  Created on: 02.01.2020
 *      Author: michi
 */

#include "Light.h"
#include <lib/yrenderer/scene/CameraParams.h>
#include "../y/Entity.h"
//#include <lib/os/msg.h>

const kaba::Class *Light::_class = nullptr;

Light::Light(const color &c, float r, float t) {
	component_type = _class;
	light.init(c, r, t);
}


void Light::on_init() {
	auto o = owner;
	light.light.pos = o->pos;
	light._ang = o->ang;
	light.light.dir = o->ang * vec3::EZ;
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
