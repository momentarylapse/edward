//
// Created by michi on 10/14/25.
//

#pragma once

#include <y/System.h>
#include <lib/base/optional.h>
#include <lib/pattern/Observable.h>

#include "World.h"

struct EntityMessageParams;
class Link;
class CollisionData;

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

class PhysicsSimulation : public obs::Node<System> {
public:
	explicit PhysicsSimulation(World* world);
	~PhysicsSimulation() override;

	obs::xsink<EntityMessageParams> in_add_component{this, &PhysicsSimulation::on_add_component};
	obs::xsink<EntityMessageParams> in_remove_component{this, &PhysicsSimulation::on_remove_component};

	void on_add_component(const EntityMessageParams& params);
	void on_remove_component(const EntityMessageParams& params);

	void on_iterate(float dt) override;

	void add_link(Link *l);
	void set_active_physics(Entity *o, bool active, bool passive);
	void register_body(SolidBody* sb);
	void unregister_body(SolidBody* sb);
	void update_all_bullet();

	base::optional<CollisionData> trace(const vec3 &p1, const vec3 &p2, int mode, Entity *o_ignore = nullptr);

	World* world;

	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;
};

