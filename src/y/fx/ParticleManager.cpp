/*
 * ParticleManager.cpp
 *
 *  Created on: Aug 9, 2020
 *      Author: michi
 */


#include "ParticleManager.h"
#include "Particle.h"
#include "Beam.h"
#include "ParticleEmitter.h"
#include <y/Entity.h>
#include <y/ComponentManager.h>


void LegacyParticleGroup::add(LegacyParticle *p) {
	if (p->is_beam)
		beams.add((LegacyBeam*)p);
	else
		particles.add(p);
}

void rebuild_legacy_groups(Array<LegacyParticleGroup>& groups) {
	for (auto& g: groups)
		g.particles.clear();

	auto& list = ComponentManager::get_list_family<LegacyParticle>();
	for (auto p: list) {
		bool found = false;
		for (auto& g: groups)
			if (g.texture == p->texture) {
				g.add(p);
				found = true;
				break;
			}
		if (!found) {
			LegacyParticleGroup g;
			g.texture = p->texture.get();
			g.add(p);
			groups.add(g);
		}
	}
}


void ParticleManager::register_particle_group(ParticleGroup *g) {
	particle_groups.add(g);
}

bool ParticleManager::unregister_particle_group(ParticleGroup *g) {
	int index = particle_groups.find(g);
	if (index >= 0) {
		particle_groups.erase(index);
		return true;
	}
	return false;
}

void ParticleManager::clear() {
	legacy_groups.clear();
}

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

	for (auto g: particle_groups)
		g->on_iterate(dt);

	rebuild_legacy_groups(legacy_groups);
}


