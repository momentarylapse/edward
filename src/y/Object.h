/*----------------------------------------------------------------------------*\
| Object                                                                       |
| -> physical entities of a model in the game                                  |
| -> manages physics on its own                                                |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2009.12.03 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#pragma once

#include "Model.h"


class Object : public Model {
public:
	Object();
//	~Object();
	void update_matrix();

	void _cdecl update_data(); // script...

	void update_theta();
	void do_physics(float dt);

	void _cdecl add_force(const vector &f, const vector &rho);
	void _cdecl add_impulse(const vector &p, const vector &rho);
	void _cdecl add_torque(const vector &t);
	void _cdecl add_torque_impulse(const vector &l);

	void _cdecl make_visible(bool visible);
	void update_motion();
	void update_mass();
};


