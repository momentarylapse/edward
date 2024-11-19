/*
 * ParticleManager.cpp
 *
 *  Created on: Aug 9, 2020
 *      Author: michi
 */


#include "ParticleManager.h"
#include "Particle.h"
#include "ParticleEmitter.h"
#include <y/Entity.h>
#include <y/ComponentManager.h>



static void iterate_legacy_particles(Array<LegacyParticle*>& particles, float dt) {
	Array<LegacyParticle*> to_del;
	foreachi (auto p, particles, i) {
		p->owner->pos += p->vel * dt;
		if (p->time_to_live >= 0) {
			p->time_to_live -= dt;
			if (p->time_to_live < 0) {
				//msg_write("PARTICLE SUICIDE  " + p->component_type->name);
				to_del.add(p);
				continue;
			}
		}
		p->on_iterate(dt);
	}

	for (auto p: to_del)
		p->owner->delete_component(p);
}

void ParticleManager::iterate(float dt) {
	auto& list = ComponentManager::get_list_family<LegacyParticle>();
	iterate_legacy_particles(list, dt);
}


