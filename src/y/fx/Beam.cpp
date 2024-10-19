/*
 * Beam.cpp
 *
 *  Created on: Aug 8, 2020
 *      Author: michi
 */

#include "Beam.h"
#include "../graphics-impl.h"

Beam::Beam(const vec3 &_pos, const vec3 &_length, const color& _col, float _r, float _ttl) : Particle(_pos, _col, _r, _ttl) {
	length = _length;
}


const kaba::Class* LegacyBeam::_class = nullptr;

LegacyBeam::LegacyBeam() {
	length = vec3::EZ;
	is_beam = true;
}

void LegacyBeam::__init_beam__() {
	new(this) LegacyBeam();
}

