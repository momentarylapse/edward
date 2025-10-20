/*
 * SolidBody.h
 *
 *  Created on: Jul 13, 2021
 *      Author: michi
 */

#pragma once

#include <ecs/Component.h>
#include <lib/math/mat3.h>
#include <lib/math/vec3.h>

class Model;

class btRigidBody;

// attach AFTER [Model, Terrain, Collider]
class SolidBody : public Component {
public:
	SolidBody();
	~SolidBody() override;


	float mass, g_factor;
	mat3 theta_0;
	bool active, passive;
	bool test_collisions;


	vec3 force_int, torque_int;
	vec3 force_ext, torque_ext;

	vec3 vel, vel_surf, rot, acc;

	bool rotating, moved, frozen;
	float time_till_freeze;


	btRigidBody* body;

	void on_init() override;


	void _cdecl update_data(); // script...

	void get_theta_world(mat3 &theta_world, mat3 &theta_world_inv);
	void do_simple_physics(float dt);

	void _cdecl add_force(const vec3 &f, const vec3 &rho);
	void _cdecl add_impulse(const vec3 &p, const vec3 &rho);
	void _cdecl add_torque(const vec3 &t);
	void _cdecl add_torque_impulse(const vec3 &l);

	void _cdecl make_visible(bool visible);
	void update_motion(int mask);
	void update_mass();

	void get_state_from_bullet();
	void state_to_bullet();

	static const kaba::Class *_class;
};
