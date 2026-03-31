//
// Created by michi on 10/14/25.
//

#include "Physics.h"
#include "../World.h"
#include "../components/Link.h"
#include "../Model.h"
#include "../components/Collider.h"
#include "../components/RigidBody.h"
#include <ecs/EntityManager.h>
#include <ecs/Entity.h>
#include <lib/base/iter.h>

#include "lib/kaba/syntax/Class.h"
#include "lib/os/msg.h"

#if HAS_LIB_BULLET
#include <btBulletDynamicsCommon.h>
//#include <BulletCollision/CollisionShapes/btConvexPointCloudShape.h>
#endif

const kaba::Class* Physics::_class = nullptr;

#if HAS_LIB_BULLET
quaternion bt_get_q(const btQuaternion &q) {
	quaternion r;
	r.x = q.x();
	r.y = q.y();
	r.z = q.z();
	r.w = q.w();
	return r;
}

vec3 bt_get_v(const btVector3 &v) {
	vec3 r;
	r.x = v.x();
	r.y = v.y();
	r.z = v.z();
	return r;
}

btVector3 bt_set_v(const vec3 &v) {
	return btVector3(v.x, v.y, v.z);
}

btQuaternion bt_set_q(const quaternion &q) {
	return btQuaternion(q.x, q.y, q.z, q.w);
}

btTransform bt_set_trafo(const vec3 &p, const quaternion &q) {
	btTransform trafo;
	trafo.setIdentity();
	trafo.setOrigin(bt_set_v(p));
	trafo.setRotation(bt_set_q(q));
	return trafo;
}
#endif

void send_collision(RigidBody *a, const CollisionData &col) {
	for (auto c: a->owner->components)
		c->on_collide(col);
}

#if HAS_LIB_BULLET
void myTickCallback(btDynamicsWorld *world, btScalar timeStep) {
	auto dispatcher = world->getDispatcher();
	int n = dispatcher->getNumManifolds();
	//CollisionData col;
	for (int i=0; i<n; i++) {
		auto contactManifold = dispatcher->getManifoldByIndexInternal(i);
		auto obA = const_cast<btCollisionObject*>(contactManifold->getBody0());
		auto obB = const_cast<btCollisionObject*>(contactManifold->getBody1());
		auto a = static_cast<RigidBody*>(obA->getUserPointer());
		auto b = static_cast<RigidBody*>(obB->getUserPointer());
		int np = contactManifold->getNumContacts();
		for (int j=0; j<np; j++) {
			auto &pt = contactManifold->getContactPoint(j);
			if (pt.getDistance() <= 0) {
				if (a->dynamic)
					send_collision(a, {b->owner, b, bt_get_v(pt.m_positionWorldOnB), bt_get_v(pt.m_normalWorldOnB)});
				if (b->dynamic)
					send_collision(b, {a->owner, a, bt_get_v(pt.m_positionWorldOnA), -bt_get_v(pt.m_normalWorldOnB)});
			}
		}
	}
}
#endif

Physics::Physics() {
	gravity = v_0;
	speed_of_sound = 1000;

	mode = PhysicsMode::FULL_EXTERNAL;
	enabled = false;
	collisions_enabled = true;
	num_steps = 10;
	num_link_steps = 5;
}

void Physics::on_init() {
#if HAS_LIB_BULLET
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	overlappingPairCache = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	dynamicsWorld->setInternalTickCallback(myTickCallback);
#endif
}

Physics::~Physics() {
#if HAS_LIB_BULLET
	delete dynamicsWorld;
	delete solver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;
#endif
}

void Physics::on_add_component(const ecs::MessageParams &params) {
	if (auto sb = params.component->as<RigidBody>()) {
		//msg_error("ADD SOLID BODY");
		register_body(sb);
	} else if (auto l = params.component->as<Link>()) {
		//msg_error("ADD LINK");
		register_link(l);
	}
}

void Physics::on_remove_component(const ecs::MessageParams &params) {
	if (auto sb = params.component->as<RigidBody>()) {
		unregister_body(sb);
	} else if (auto l = params.component->as<Link>()) {
		unregister_link(l);
	}
}



void Physics::on_iterate(float dt) {
	if (!enabled)
		return;

	auto& list = entity_manager->get_component_list<RigidBody>();

	if (mode == PhysicsMode::FULL_EXTERNAL) {
#if HAS_LIB_BULLET
		dynamicsWorld->setGravity(bt_set_v(gravity));
		dynamicsWorld->stepSimulation(dt, 10);

		for (auto *o: list)
			o->get_state_from_bullet();
#endif
	} else if (mode == PhysicsMode::SIMPLE) {
		for (auto *o: list)
			o->do_simple_physics(dt);
	}
}

void Physics::register_link(Link *l) {
	l->create();
#if HAS_LIB_BULLET
	dynamicsWorld->addConstraint(l->con, true);
#endif
}

void Physics::unregister_link(Link *l) {

}

void Physics::register_body(RigidBody *sb) {
#if HAS_LIB_BULLET
	dynamicsWorld->addRigidBody(sb->body);
#endif
}

void Physics::unregister_body(RigidBody *sb) {
#if HAS_LIB_BULLET
	dynamicsWorld->removeRigidBody(sb->body);
#endif
}



void Physics::set_dynamic(ecs::Entity *o, bool dynamic) {
	auto sb = o->get_component<RigidBody>();
	auto c = o->get_component_derived<Collider>();

#if HAS_LIB_BULLET
	btScalar mass(dynamic ? sb->mass : 0);
	btVector3 local_inertia(0, 0, 0);
	if (c->col_shape) {
		c->col_shape->calculateLocalInertia(mass, local_inertia);
		sb->theta_0._00 = local_inertia.x();
		sb->theta_0._11 = local_inertia.y();
		sb->theta_0._22 = local_inertia.z();
	}
	sb->body->setMassProps(mass, local_inertia);

	/*if (!passive and test_collisions) {
		msg_error("FIXME pure collision");
		dynamicsWorld->addCollisionObject(o->body);
	}*/
#endif


	sb->dynamic = dynamic;
}

base::optional<CollisionData> Physics::trace(const vec3 &p1, const vec3 &p2, int mode, ecs::Entity *o_ignore) {
#if HAS_LIB_BULLET
	btCollisionWorld::ClosestRayResultCallback ray_callback(bt_set_v(p1), bt_set_v(p2));
	//ray_callback.m_collisionFilterMask = FILTER_CAMERA;

	// Perform raycast
	this->dynamicsWorld->getCollisionWorld()->rayTest(bt_set_v(p1), bt_set_v(p2), ray_callback);
	if (ray_callback.hasHit()) {
		CollisionData d;
		auto sb = static_cast<RigidBody *>(ray_callback.m_collisionObject->getUserPointer());
		d.pos = bt_get_v(ray_callback.m_hitPointWorld);
		d.n = bt_get_v(ray_callback.m_hitNormalWorld);
		d.entity = sb->owner;
		d.body = sb;

		// ignore...
		if (sb and sb->owner == o_ignore) {
			vec3 dir = (p2 - p1).normalized();
			return trace(d.pos + dir * 2, p2, mode, o_ignore);
		}
		return d;
	}
#endif
	return base::None;
}

void Physics::update_all_bullet() {
	for (auto &sb: entity_manager->get_component_list<RigidBody>())
		sb->state_to_bullet();
}

vec3 Physics::get_g(const vec3 &pos) const {
	return gravity;
}




