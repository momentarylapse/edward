/*
 * Light.cpp
 *
 *  Created on: 02.01.2020
 *      Author: michi
 */

#include "Light.h"
#include <lib/yrenderer/scene/CameraParams.h>
#include <ecs/Entity.h>
//#include <lib/os/msg.h>

const kaba::Class *Light::_class = nullptr;

Light::Light(yrenderer::LightType type, const color &c, float theta) {
	component_type = _class;
	light.init(type, c, theta);
}


void Light::on_init() {
	auto o = owner;
	light.pos = o->pos;
	light._ang = o->ang;
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
