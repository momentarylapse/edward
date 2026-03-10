//
// Created by michi on 10/14/25.
//

#pragma once

#include <ecs/System.h>
#include <ecs/EntityManager.h>
#include <lib/base/optional.h>

class Link;
class CollisionData;
class SolidBody;

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;


enum class PhysicsMode {
	NONE,
	SIMPLE,
	FULL_INTERNAL,
	FULL_EXTERNAL,
};

class Physics : public System {
public:
	explicit Physics();
	~Physics() override;

	void on_add_component(const EntityMessageParams& params) override;
	void on_remove_component(const EntityMessageParams& params) override;

	void on_init() override;
	void on_iterate(float dt) override;

	void add_link(Link *l);
	void delete_link(Link *l);

	void set_active_physics(Entity *o, bool active, bool passive);
	void register_body(SolidBody* sb);
	void unregister_body(SolidBody* sb);
	void update_all_bullet();

	base::optional<CollisionData> trace(const vec3 &p1, const vec3 &p2, int mode, Entity *o_ignore = nullptr);

	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;

	vec3 get_g(const vec3 &pos) const;

	bool enabled;
	bool collisions_enabled;
	PhysicsMode mode;

	float speed_of_sound;

	vec3 gravity;

	int num_steps, num_link_steps;

	Array<Link*> links;

	static const kaba::Class* _class;
};

