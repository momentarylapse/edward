/*
 * ParticleEmitter.cpp
 *
 *  Created on: 19 Apr 2022
 *      Author: michi
 */

#include "ParticleEmitter.h"
#include "Particle.h"
#include <lib/ygraphics/graphics-impl.h>
#include <y/Entity.h>
#include <lib/math/math.h>
#include <lib/math/random.h>
#include <lib/os/msg.h>
#include <y/EngineData.h>
#include <lib/yrenderer/Context.h>

static Random pe_random;

const kaba::Class *ParticleGroup::_class = nullptr;
const kaba::Class *ParticleEmitter::_class = nullptr;

ParticleGroup::ParticleGroup() {
	if (engine.context->tex_white)
		texture = engine.context->tex_white;
	source = rect::ID;
	//pos = vec3::ZERO;
}

void ParticleGroup::__init__() {
	new(this) ParticleGroup;
}


Particle* ParticleGroup::emit_particle(const vec3& pos, const color& col, float r, float ttl) {
	for (auto& p: particles)
		if (!p.enabled) {
			p = Particle(pos, col, r, ttl);
			return &p;
		}
	particles.add(Particle(pos, col, r, ttl));
	return &particles.back();
}

Beam* ParticleGroup::emit_beam(const vec3& pos, const vec3& length, const color& col, float r, float ttl) {
	beams.add(Beam(pos, length, col, r, ttl));
	return &beams.back();
}

void ParticleGroup::on_iterate(float dt) {
	iterate_particles(dt);
}

void ParticleGroup::iterate_particles(float dt) {
	for (auto &p: particles) {
		on_iterate_particle(&p, dt);
		p.pos += p.vel * dt;
	}
	for (auto &b: beams) {
		on_iterate_beam(&b, dt);
		b.pos += b.vel * dt;
	}

	for (int i=0; i<particles.num; i++) {
		auto& p = particles[i];
		//p->pos += p->vel * dt;
		p.time_to_live -= dt;
		if (p.time_to_live < 0 and p.suicidal) {
			p.enabled = false;
			//particles.erase(i);
			//i --;
		}
	}
	for (int i=0; i<beams.num; i++) {
		auto& b = beams[i];
		//p->pos += p->vel * dt;
		b.time_to_live -= dt;
		if (b.time_to_live < 0 and b.suicidal) {
			beams.erase(i);
			i --;
		}
	}
}


ParticleEmitter::ParticleEmitter() {
	spawn_time_to_live = 1;
	spawn_dt = 0.1f;

	spawn_vel = vec3(0,0,100);
	spawn_dvel = 20;
	spawn_radius = 10;
	spawn_dradius = 5;
}

void ParticleEmitter::__init__() {
	new(this) ParticleEmitter;
}
void ParticleEmitter::on_iterate(float dt) {
	iterate_emitter(dt);
}

void ParticleEmitter::iterate_emitter(float dt) {
	tt += dt;
	while (tt >= spawn_dt) {
		tt -= spawn_dt;
		auto p = emit_particle(owner->pos, White, spawn_radius, spawn_time_to_live);
		on_init_particle(p);
		p->pos += p->vel * tt;
	}

	iterate_particles(dt);
	//ParticleGroup::on_iterate(dt);
}

