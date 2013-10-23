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
	reset();
	name = "-terrain-";
	visible = false;
	mass = 10000.0f;
	mass_inv = 0;
	radius = 30000000;
	Matrix3Identity( theta_0 );
	Matrix3Identity( theta );
	memset(&theta_inv, 0, sizeof(theta_inv));
	MatrixIdentity(_matrix);
	//theta = theta * 10000000.0f;
	g_factor = 1;
	active_physics = false;
	passive_physics = true;
	rotating = false;
	frozen = true;
	time_till_freeze = -1;
	//SpecialID=IDFloor;
	//UpdateMatrix();
	//UpdateTheta();
	//msg_left();
}

void Object::AddForce(const vector &f, const vector &rho)
{
	if (Engine.Elapsed<=0)
		return;
	if (!active_physics)
		return;
	force_ext += f;
	torque_ext += VecCrossProduct(rho, f);
	//TestVectorSanity(f, "f addf");
	//TestVectorSanity(rho, "rho addf");
	//TestVectorSanity(torque, "Torque addf");
	unfreeze(this);
}

void Object::AddTorque(const vector &t)
{
	if (Engine.Elapsed<=0)
		return;
	if (!active_physics)
		return;
	torque_ext += t;
	//TestVectorSanity(Torque,"Torque addt");
	unfreeze(this);
}

void Object::MakeVisible(bool _visible_)
{
	if (_visible_ == visible)
		return;
	if (_visible_)
		GodRegisterModel((Model*)this);
	else
		GodUnregisterModel((Model*)this);
	visible = _visible_;
}

void Object::DoPhysics()
{
	if (Engine.Elapsed<=0)
		return;
	msg_db_f("object::DoPhysics",2);
	msg_db_m(name.c_str(),3);
	

	if (_vec_length_fuzzy_(force_int) * mass_inv > AccThreshold)
	{unfreeze(this);}

	if ((active_physics) && (!frozen)){

		if (inf_v(pos))	msg_error("inf   CalcMove Pos  1");
		if (inf_v(vel))	msg_error("inf   CalcMove Vel  1");

			// linear acceleration
			acc = force_int * mass_inv;

			// integrate the equations of motion.... "euler method"
			vel += acc * Engine.Elapsed;
			pos += vel * Engine.Elapsed;

		if (inf_v(acc))	msg_error("inf   CalcMove Acc");
		if (inf_v(vel))	msg_error("inf   CalcMove Vel  2");
		if (inf_v(pos))	msg_error("inf   CalcMove Pos  2");

		//}

		// rotation
		if ((rot != v_0) || (torque_int != v_0)){

			quaternion q_dot, q_w;
			q_w = quaternion( 0, rot );
			q_dot = 0.5f * q_w * ang;
			ang += q_dot * Engine.Elapsed;
			ang.normalize();

			#ifdef _realistic_calculation_
				vector L = theta * rot + torque_int * Engine.Elapsed;
				UpdateTheta();
				rot = theta_inv * L;
			#else
				UpdateTheta();
				rot += theta_inv * torque_int * Engine.Elapsed;
			#endif
		}
	}

	// new orientation
	UpdateMatrix();
	UpdateTheta();

	_ResetPhysAbsolute_();

	// reset forces
	force_int = torque_int = v_0;

	// did anything change?
	moved = false;
	//if ((Pos!=Pos_old)||(ang!=ang_old))
	//if ( (vel_surf!=v_0) || (VecLengthFuzzy(Pos-Pos_old)>2.0f*Elapsed) )//||(VecAng!=ang_old))
	if (active_physics){
		if ( (vel_surf != v_0) || (_vec_length_fuzzy_(vel) > VelThreshold) || (_vec_length_fuzzy_(rot) * radius > VelThreshold))
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
		time_till_freeze -= Engine.Elapsed;
		if (time_till_freeze < 0){
			frozen = true;
			force_ext = torque_ext = v_0;
		}else
			on_ground = false;
	}
}



// rotate inertia tensor into world coordinates
void Object::UpdateTheta()
{
	if (active_physics){
		matrix3 r,r_inv;
		Matrix3RotationQ(r, ang);
		Matrix3Transpose(r_inv, r);
		theta = (r * theta_0 * r_inv);
		Matrix3Inverse(theta_inv, theta);
	}else{
		// Theta and ThetaInv already = identity
		memset(&theta_inv, 0, sizeof(matrix3));
	}
}

void Object::UpdateMatrix()
{
	matrix trans,rot;
	MatrixRotationQ(rot, ang);
	MatrixTranslation(trans, pos);
	MatrixMultiply(_matrix, trans, rot);
}

// scripts have to call this after 
void Object::UpdateData()
{
	unfreeze(this);
	if (!active_physics){
		UpdateMatrix();
		UpdateTheta();
	}

	// set ode data..
}

