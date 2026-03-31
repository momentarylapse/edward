/*
 * Beam.h
 *
 *  Created on: Aug 8, 2020
 *      Author: michi
 */

#pragma once

#include "Particle.h"

struct Beam : Particle {
	Beam() {}
	Beam(const vec3 &pos, const vec3 &length, const color& col, float r, float ttl);

	vec3 length;
};


struct LegacyBeam : LegacyParticle {
	LegacyBeam();

	void __init_beam__();

	vec3 length;

	static const kaba::Class *_class;
};
