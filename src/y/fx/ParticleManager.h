/*
 * ParticleManager.h
 *
 *  Created on: Aug 9, 2020
 *      Author: michi
 */

#pragma once

#include "../graphics-fwd.h"
#include "../lib/base/base.h"

class LegacyParticle;
class LegacyBeam;
class ParticleGroup;
class vec3;

struct LegacyParticleGroup {
	Array<LegacyParticle*> particles;
	Array<LegacyBeam*> beams;
	void add(LegacyParticle *p);
	Texture *texture;
};

class ParticleManager {
public:
	Array<LegacyParticleGroup> legacy_groups;
	Array<ParticleGroup*> particle_groups;
	void register_particle_group(ParticleGroup *g);
	bool unregister_particle_group(ParticleGroup *g);
	void clear();
	void iterate(float dt);
};


