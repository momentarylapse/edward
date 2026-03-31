/*
 * ParticleManager.h
 *
 *  Created on: Aug 9, 2020
 *      Author: michi
 */

#pragma once

#include <ecs/System.h>

struct LegacyParticle;

class ParticleManager : public ecs::System {
public:
	ParticleManager();
	void on_iterate(float dt) override;
	void iterate_legacy_particles(float dt);
	static const kaba::Class* _class;
};


