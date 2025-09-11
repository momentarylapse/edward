/*
 * SolidBodyComponent.cpp
 *
 *  Created on: Jul 13, 2021
 *      Author: michi
 */

#include "SolidBody.h"
#include "Collider.h"
#include "../World.h"
#include "../Model.h"
#include "../ModelManager.h"
#include "../Terrain.h"
#include "../../y/Entity.h"
#include "../../y/EngineData.h"
#include <lib/math/quaternion.h>
#include <lib/os/msg.h>

const kaba::Class *SolidBody::_class = nullptr;


#if HAS_LIB_BULLET
#include <btBulletDynamicsCommon.h>


btVector3 bt_set_v(const vec3 &v);
btQuaternion bt_set_q(const quaternion &q);
vec3 bt_get_v(const btVector3 &v);
quaternion bt_get_q(const btQuaternion &q);
btTransform bt_set_trafo(const vec3 &p, const quaternion &q);
#endif


static int num_insane=0;


inline bool ainf_v(vec3 &v) {
	if (inf_v(v))
		return true;
	return (v.length_fuzzy() > 100000000000.0f);
}

inline bool TestVectorSanity(vec3 &v,char *name) {
	if (ainf_v(v)) {
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



SolidBody::SolidBody() {
	active = false;
	passive = false;
	mass = 0;
	body = nullptr;

	vel = rot = v_0;
	rotating  = true;
	moved = false;
	frozen = false;
	time_till_freeze = 0;

	vel = rot = v_0;
	vel_surf = acc = v_0;

	force_int = torque_int = v_0;
	force_ext = torque_ext = v_0;

	g_factor = 1;
	test_collisions = true;
}

void SolidBody::copy_data(SolidBody *source) {
	mass = source->mass;
	theta_0 = source->theta_0;
	active = source->active;
	passive = source->passive;
	test_collisions = source->test_collisions;
}


void SolidBody::on_init() {
	auto o = owner;
	auto m = owner->get_component<Model>();
	auto t = o->get_component<Terrain>();

	// import
	// TODO better place for this hack!
	if (m and m->_template->solid_body) {
		copy_data(m->_template->solid_body);
	} else if (t) {
		mass = 10000.0f;
		theta_0 = mat3::ZERO;
		passive = true;
	}

	if (!active and !passive)
		return;

#if HAS_LIB_BULLET
	btCollisionShape *col_shape = nullptr;
	if (auto col = owner->get_component_derived<Collider>())
		col_shape = col->col_shape;


	btTransform start_transform = bt_set_trafo(o->pos, o->ang);

	btScalar _mass(active ? mass : 0);
	btVector3 local_inertia(0, 0, 0);
	//if (isDynamic)
	if (col_shape) {
		col_shape->calculateLocalInertia(_mass, local_inertia);
		theta_0._00 = local_inertia.x();
		theta_0._11 = local_inertia.y();
		theta_0._22 = local_inertia.z();
	}

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* motion_state = new btDefaultMotionState(start_transform);
	btRigidBody::btRigidBodyConstructionInfo rb_info(_mass, motion_state, col_shape, local_inertia);
	if (m) {
		rb_info.m_friction = (m->material[0]->friction.sliding + m->material[0]->friction._static) / 2;
	//	rb_info.m_rollingFriction = m->material[0]->friction.rolling;
	}
	body = new btRigidBody(rb_info);

	body->setUserPointer(this);
	update_mass();

	//else if (o->test_collisions)
	//	dynamicsWorld->addCollisionObject(o->body);
#endif
}

SolidBody::~SolidBody() {
#if HAS_LIB_BULLET
	if (body) {
		delete body->getMotionState();
		delete body;
	}
#endif
}




void SolidBody::add_force(const vec3 &f, const vec3 &rho) {
	if (engine.elapsed<=0)
		return;
	if (!active)
		return;
	if (world.physics_mode == PhysicsMode::FULL_EXTERNAL) {
#if HAS_LIB_BULLET
		body->activate(); // why doesn't this happen automatically?!? bug in bullet?
		body->applyForce(bt_set_v(f), bt_set_v(rho));
#endif
	} else {
		force_ext += f;
		torque_ext += vec3::cross(rho, f);
		//TestVectorSanity(f, "f addf");
		//TestVectorSanity(rho, "rho addf");
		//TestVectorSanity(torque, "Torque addf");
		unfreeze(this);
	}
}

void SolidBody::add_impulse(const vec3 &p, const vec3 &rho) {
	if (engine.elapsed<=0)
		return;
	if (!active)
		return;
	if (world.physics_mode == PhysicsMode::FULL_EXTERNAL) {
#if HAS_LIB_BULLET
		body->activate();
		body->applyImpulse(bt_set_v(p), bt_set_v(rho));
#endif
	} else {
		vel += p / mass;
		//rot += ...;
		unfreeze(this);
	}
}

void SolidBody::add_torque(const vec3 &t) {
	if (engine.elapsed <= 0)
		return;
	if (!active)
		return;
	if (world.physics_mode == PhysicsMode::FULL_EXTERNAL) {
#if HAS_LIB_BULLET
		body->activate();
		body->applyTorque(bt_set_v(t));
#endif
	} else {
		torque_ext += t;
		//TestVectorSanity(Torque,"Torque addt");
		unfreeze(this);
	}
}

void SolidBody::add_torque_impulse(const vec3 &l) {
	if (engine.elapsed <= 0)
		return;
	if (!active)
		return;
	if (world.physics_mode == PhysicsMode::FULL_EXTERNAL) {
#if HAS_LIB_BULLET
		body->activate();
		body->applyTorqueImpulse(bt_set_v(l));
#endif
	} else {
		//rot += ...
		//TestVectorSanity(Torque,"Torque addt");
		unfreeze(this);
	}
}



void SolidBody::do_simple_physics(float dt) {
	if (dt <= 0)
		return;

	auto o = owner;


	if (active) {
		if (_vec_length_fuzzy_(force_int) > AccThreshold * mass)
		{unfreeze(this);}
	}

	if (active and !frozen) {

		if (inf_v(o->pos))	msg_error("inf   CalcMove Pos  1");
		if (inf_v(vel))	msg_error("inf   CalcMove Vel  1");

			// linear acceleration
			acc = force_int / mass;

			// integrate the equations of motion.... "euler method"
			vel += acc * dt;
			o->pos += vel * dt;

		if (inf_v(acc))	msg_error("inf   CalcMove Acc");
		if (inf_v(vel))	msg_error("inf   CalcMove Vel  2");
		if (inf_v(o->pos))	msg_error("inf   CalcMove Pos  2");

		//}

		// rotation
		if ((rot != v_0) or (torque_int != v_0)) {

			const auto q_w = quaternion( 0, rot );
			const auto q_dot = 0.5f * q_w * o->ang;
			o->ang += q_dot * dt;
			o->ang.normalize();

			mat3 theta_world, theta_world_inv;
			get_theta_world(theta_world, theta_world_inv);

			#ifdef _realistic_calculation_

				vec3 L = theta_world * rot + torque_int * dt;
				rot = theta_world_inv * L;
			#else
				rot += theta_world_inv * torque_int * dt;
			#endif
		}
	}

	// new orientation
	auto m = owner->get_component<Model>();
	if (m) {
		m->update_matrix();

		m->_ResetPhysAbsolute_();
	}

	// reset forces
	force_int = torque_int = v_0;

	// did anything change?
	moved = false;
	float rr = m ? m->prop.radius : 1.0f;
	//if ((Pos!=Pos_old)or(ang!=ang_old))
	//if ( (vel_surf!=v_0) or (VecLengthFuzzy(Pos-Pos_old)>2.0f*Elapsed) )//or(VecAng!=ang_old))
	if (active) {
		if ( (vel_surf != v_0) or (_vec_length_fuzzy_(vel) > VelThreshold) or (_vec_length_fuzzy_(rot) * rr > VelThreshold))
			moved = true;
	} else {
		frozen = true;
	}
	// would be best to check on the sub models....
	/*if (model)
		if (model->bone.num > 0)
			moved = true;*/

	if (moved) {
		unfreeze(this);
	} else if (!frozen) {
		time_till_freeze -= dt;
		if (time_till_freeze < 0) {
			frozen = true;
			force_ext = torque_ext = v_0;
		}
	}
}



// rotate inertia tensor into world coordinates
void SolidBody::get_theta_world(mat3 &theta_world, mat3 &theta_world_inv) {
	auto r = mat3::rotation(owner->ang);
	auto r_inv = r.transpose();
	theta_world = (r * theta_0 * r_inv);

	if (active) {
		theta_world_inv = theta_world.inverse();
	} else {
		// Theta and ThetaInv already = identity
		theta_world_inv = mat3::ZERO;
	}
}



// scripts have to call this after
void SolidBody::update_data() {
	unfreeze(this);
	if (!active) {
		owner->get_component<Model>()->update_matrix();
	}

	// set ode data..
}


void SolidBody::update_motion(int mask) {
#if HAS_LIB_BULLET
	auto o = owner;
	btTransform trans;
	if (mask & 1)
		body->getWorldTransform().setOrigin(bt_set_v(o->pos));
	if (mask & 2)
		body->getWorldTransform().setRotation(bt_set_q(o->ang));
	if (mask & 4)
		body->setLinearVelocity(bt_set_v(vel));
	if (mask & 8)
		body->setAngularVelocity(bt_set_v(rot));
#endif
}

void SolidBody::update_mass() {
#if HAS_LIB_BULLET
	if (active) {
		btScalar _mass(mass);
		btVector3 local_inertia(theta_0._00, theta_0._11, theta_0._22);
		//if (colShape)
		//	colShape->calculateLocalInertia(mass, localInertia);
		body->setMassProps(_mass, local_inertia);
	} else {
		//btScalar _mass(0);
		btVector3 local_inertia(0, 0, 0);
		//body->setMassProps(mass, localInertia);
	}
#endif
}

void SolidBody::get_state_from_bullet() {
#if HAS_LIB_BULLET
	if (active) {
		btTransform trans;
		body->getMotionState()->getWorldTransform(trans);
		owner->pos = bt_get_v(trans.getOrigin());
		owner->ang = bt_get_q(trans.getRotation());
		vel = bt_get_v(body->getLinearVelocity());
		rot = bt_get_v(body->getAngularVelocity());
	} else {

	}
#endif
}

void SolidBody::state_to_bullet() {
#if HAS_LIB_BULLET
	if (active) {
		btTransform trans;
		trans.setOrigin(bt_set_v(owner->pos));
		trans.setRotation(bt_set_q(owner->ang));
		body->getMotionState()->setWorldTransform(trans);
		body->setLinearVelocity(bt_set_v(vel));
		body->setAngularVelocity(bt_set_v(rot));
	} else {

	}
#endif
}
