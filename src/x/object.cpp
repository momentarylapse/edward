/*----------------------------------------------------------------------------*\
| Object                                                                       |
| -> physical entities of a model in the game                                  |
| -> manages physics on its own                                                |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2008.10.26 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#include "object.h"
#include "world.h"
#include "material.h"
#include "../meta.h"
#include "../lib/file/file.h"

static int num_insane=0;

inline bool ainf_v(vector &v)
{
	if (inf_v(v))
		return true;
	return (v.length_fuzzy() > 100000000000.0f);
}

inline bool TestVectorSanity(vector &v,char *name)
{
	if (ainf_v(v)){
		num_insane++;
		v=v_0;
		if (num_insane>100)
			return false;
		msg_error(format("Vektor %s unendlich!!!!!!!",name));
		return true;
	}
	return false;
}



#define _realistic_calculation_


#define VelThreshold			1.0f
#define AccThreshold			10.0f
#define MaxTimeTillFreeze		1.0f

#define unfreeze(object)	(object)->time_till_freeze = MaxTimeTillFreeze; (object)->frozen = false


// neutral object (for terrains,...)
Object::Object()
{
	//msg_right();
	//msg_write("terrain object");
	material.add(new Material);
	script_data.name = "-terrain-";
	visible = false;
	physics_data.mass = 10000.0f;
	physics_data.mass_inv = 0;
	prop.radius = 30000000;
	physics_data.theta_0 = matrix3::ID;
	physics_data.theta = matrix3::ID;
	memset(&physics_data.theta_inv, 0, sizeof(physics_data.theta_inv));
	_matrix = matrix::ID;
	//theta = theta * 10000000.0f;
	physics_data.g_factor = 1;
	physics_data.active = false;
	physics_data.passive = true;
	rotating = false;
	frozen = true;
	time_till_freeze = -1;
	//SpecialID=IDFloor;
	//UpdateMatrix();
	//UpdateTheta();
	//msg_left();
}

void Object::add_force(const vector &f, const vector &rho) {
	if (engine.elapsed<=0)
		return;
	if (!physics_data.active)
		return;
	force_ext += f;
	torque_ext += vector::cross(rho, f);
	//TestVectorSanity(f, "f addf");
	//TestVectorSanity(rho, "rho addf");
	//TestVectorSanity(torque, "Torque addf");
	unfreeze(this);
}

void Object::add_torque(const vector &t) {
	if (engine.elapsed <= 0)
		return;
	if (!physics_data.active)
		return;
	torque_ext += t;
	//TestVectorSanity(Torque,"Torque addt");
	unfreeze(this);
}

void Object::make_visible(bool _visible_) {
	if (_visible_ == visible)
		return;
	if (_visible_)
		world.register_model(this);
	else
		world.unregister_model(this);
	visible = _visible_;
}

void Object::do_physics(float dt)
{
	if (dt <= 0)
		return;
	

	if (_vec_length_fuzzy_(force_int) * physics_data.mass_inv > AccThreshold)
	{unfreeze(this);}

	if ((physics_data.active) and (!frozen)){

		if (inf_v(pos))	msg_error("inf   CalcMove Pos  1");
		if (inf_v(vel))	msg_error("inf   CalcMove Vel  1");

			// linear acceleration
			acc = force_int * physics_data.mass_inv;

			// integrate the equations of motion.... "euler method"
			vel += acc * dt;
			pos += vel * dt;

		if (inf_v(acc))	msg_error("inf   CalcMove Acc");
		if (inf_v(vel))	msg_error("inf   CalcMove Vel  2");
		if (inf_v(pos))	msg_error("inf   CalcMove Pos  2");

		//}

		// rotation
		if ((rot != v_0) or (torque_int != v_0)){

			quaternion q_dot, q_w;
			q_w = quaternion( 0, rot );
			q_dot = 0.5f * q_w * ang;
			ang += q_dot * dt;
			ang.normalize();

			#ifdef _realistic_calculation_
				vector L = physics_data.theta * rot + torque_int * dt;
				update_theta();
				rot = physics_data.theta_inv * L;
			#else
				UpdateTheta();
				rot += theta_inv * torque_int * dt;
			#endif
		}
	}

	// new orientation
	update_matrix();
	update_theta();

	_ResetPhysAbsolute_();

	// reset forces
	force_int = torque_int = v_0;

	// did anything change?
	moved = false;
	//if ((Pos!=Pos_old)or(ang!=ang_old))
	//if ( (vel_surf!=v_0) or (VecLengthFuzzy(Pos-Pos_old)>2.0f*Elapsed) )//or(VecAng!=ang_old))
	if (physics_data.active){
		if ( (vel_surf != v_0) or (_vec_length_fuzzy_(vel) > VelThreshold) or (_vec_length_fuzzy_(rot) * prop.radius > VelThreshold))
			moved = true;
	}else{
		frozen = true;
	}
	// would be best to check on the sub models....
	/*if (model)
		if (model->bone.num > 0)
			moved = true;*/

	if (moved){
		unfreeze(this);
		on_ground=false;
	}else if (!frozen){
		time_till_freeze -= dt;
		if (time_till_freeze < 0){
			frozen = true;
			force_ext = torque_ext = v_0;
		}else
			on_ground = false;
	}
}



// rotate inertia tensor into world coordinates
void Object::update_theta() {
	if (physics_data.active){
		auto r = matrix3::rotation_q( ang);
		auto r_inv = r.transpose();
		physics_data.theta = (r * physics_data.theta_0 * r_inv);
		physics_data.theta_inv = physics_data.theta.inverse();
	}else{
		// Theta and ThetaInv already = identity
		memset(&physics_data.theta_inv, 0, sizeof(matrix3));
	}
}

void Object::update_matrix() {
	auto rot = matrix::rotation_q(ang);
	auto trans = matrix::translation(pos);
	_matrix = trans * rot;
}

// scripts have to call this after 
void Object::update_data() {
	unfreeze(this);
	if (!physics_data.active){
		update_matrix();
		update_theta();
	}

	// set ode data..
}
