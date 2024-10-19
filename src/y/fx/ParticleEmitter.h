/*
 * ParticleEmitter.h
 *
 *  Created on: 19 Apr 2022
 *      Author: michi
 */

#ifndef SRC_FX_PARTICLEEMITTER_H_
#define SRC_FX_PARTICLEEMITTER_H_

#include "../graphics-fwd.h"
#include "../lib/base/base.h"
#include "../lib/base/pointer.h"
#include "../lib/math/vec3.h"
#include "../lib/image/color.h"
#include "../y/BaseClass.h"
#include "../y/Component.h"
#include "Particle.h"
#include "Beam.h"

class ParticleGroup : public Component {
public:
	ParticleGroup();
	void __init__();

	Particle* emit_particle(const vec3& pos, const color& col, float r, float ttl);
	Beam* emit_beam(const vec3& pos, const vec3& length, const color& col, float r, float ttl);
	virtual void on_iterate_particle(Particle *p, float dt) {}
	virtual void on_iterate_beam(Beam *b, float dt) {}
	void on_iterate(float dt) override;
	void iterate_particles(float dt);

	//shared<Texture> texture;
	Texture* texture;
	rect source;

	Array<Particle> particles;
	Array<Beam> beams;

	static const kaba::Class *_class;
};

class ParticleEmitter : public ParticleGroup {
public:
	ParticleEmitter();
	void __init__();

	virtual void on_init_particle(Particle *p) {}
	virtual void on_init_beam(Beam *b) {}
	void on_iterate(float dt) override;
	void iterate_emitter(float dt);

	float spawn_time_to_live;
	float tt = 0;
	bool spawn_beams = false;
	float spawn_dt;
	//int next_index = 0;
	vec3 spawn_vel;
	float spawn_dvel;
	float spawn_radius;
	float spawn_dradius;

	static const kaba::Class *_class;
};

#endif /* SRC_FX_PARTICLEEMITTER_H_ */
