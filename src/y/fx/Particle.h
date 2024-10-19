/*
 * Particle.h
 *
 *  Created on: 08.01.2020
 *      Author: michi
 */

#pragma once

#include "../graphics-fwd.h"
#include "../lib/base/base.h"
#include "../lib/base/pointer.h"
#include "../lib/math/vec3.h"
#include "../lib/math/rect.h"
#include "../lib/image/color.h"
#include <y/BaseClass.h>
#include <y/Component.h>


class Particle {
public:
	Particle() {}
	Particle(const vec3 &pos, const color& col, float r, float ttl);

	vec3 pos;
	vec3 vel;
	color col;
	float radius;
	float time_to_live;
	bool suicidal;
	bool enabled;
};

class LegacyParticle : public Component {
public:
	LegacyParticle();
	~LegacyParticle() override;

	void __init__();
	void __delete__() override;
	void on_iterate(float dt) override {}

	vec3 vel;
	color col;
	rect source;
	shared<Texture> texture;
	float radius;
	float time_to_live;
	bool enabled, is_beam;

	static const kaba::Class *_class;
};

