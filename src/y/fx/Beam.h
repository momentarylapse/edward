/*
 * Beam.h
 *
 *  Created on: Aug 8, 2020
 *      Author: michi
 */

#pragma once

#include "Particle.h"

class Beam : public Particle {
public:
	Beam() {}
	Beam(const vec3 &pos, const vec3 &length, const color& col, float r, float ttl);

	vec3 length;
};


class LegacyBeam : public LegacyParticle {
public:
	LegacyBeam();

	void __init_beam__();

	vec3 length;

	static const kaba::Class *_class;
};
