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
#include <lib/yrenderer/scene/Light.h>
#include <ecs/Component.h>

namespace yrenderer {
	struct CameraParams;
}

class Light : public Component {
public:
	Light(yrenderer::LightType type, const color &c, float theta = -1);
	/*void __init_parallel__(const quaternion &ang, const color &c);
	void __init_spherical__(const vec3 &p, const color &c, float r);
	void __init_cone__(const vec3 &p, const quaternion &ang, const color &c, float r, float t);*/

	void on_init() override;

	void set_direction(const vec3 &dir);

	yrenderer::Light light;

	//yrenderer::LightType type() const;

	static const kaba::Class *_class;
};


