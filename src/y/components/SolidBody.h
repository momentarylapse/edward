/*
 * SolidBody.h
 *
 *  Created on: Jul 13, 2021
 *      Author: michi
 */

#pragma once

#include "../../y/Component.h"
#include "../../lib/math/matrix3.h"
#include "../../lib/math/vector.h"

class Model;

class btRigidBody;


class SolidBody : public Component {
public:
	SolidBody();
	virtual ~SolidBody();

	void copy_data(SolidBody *source);


	float mass, g_factor;
	matrix3 theta_0;
	bool active, passive;
	bool test_collisions;


	vector force_int, torque_int;
	vector force_ext, torque_ext;

	vector vel, vel_surf, rot, acc;

	bool rotating, moved, frozen;
	float time_till_freeze;


	btRigidBody* body;

	void on_init() override;


	void _cdecl update_data(); // script...

	void get_theta_world(matrix3 &theta_world, matrix3 &theta_world_inv);
	void do_simple_physics(float dt);

	void _cdecl add_force(const vector &f, const vector &rho);
	void _cdecl add_impulse(const vector &p, const vector &rho);
	void _cdecl add_torque(const vector &t);
	void _cdecl add_torque_impulse(const vector &l);

	void _cdecl make_visible(bool visible);
	void update_motion(int mask);
	void update_mass();

	void get_state_from_bullet();

	static const kaba::Class *_class;
};
