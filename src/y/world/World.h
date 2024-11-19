/*----------------------------------------------------------------------------*\
| God                                                                          |
| -> manages objetcs and interactions                                          |
| -> loads and stores the world data (level)                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2008.12.06 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#pragma once


#include <lib/base/base.h>
#include <lib/base/optional.h>
#include <lib/base/pointer.h>
#include <lib/base/callable.h>
#include <lib/os/path.h>
#include <lib/image/color.h>
#include "LevelData.h"


class Model;
class Object;
class Material;
class Terrain;
class SolidBody;
class MultiInstance;
class Entity;
class TemplateDataScriptVariable;
class Light;
class Camera;
class ParticleManager;
class LegacyParticle;
class Link;
class LevelData;
enum class LinkType;


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


// network messages
struct GodNetMessage {
	int msg, arg_i[4];
	string arg_s;
};

class CollisionData {
public:
	Entity *entity;
	SolidBody *body;
	vec3 pos, n;
};


// game data
class World {
public:
	World();
	~World();
	void reset();
	bool load(const LevelData &ld);

	Path next_filename;
	void load_soon(const Path &filename);
	void save(const Path &filename);

	Entity *create_entity(const vec3 &pos, const quaternion &ang);
	Array<Entity*> entities;
	void register_entity(Entity *e);
	void unregister_entity(Entity *e);

	Model *create_object(const Path &filename, const vec3 &pos, const quaternion &ang);
	Model *create_object_no_reg(const Path &filename, const vec3 &pos, const quaternion &ang);
	Model *create_object_no_reg_x(const Path &filename, const string &name, const vec3 &pos, const quaternion &ang);
	Terrain *create_terrain(const Path &filename, const vec3 &pos);
	Terrain *create_terrain_no_reg(const Path &filename, const vec3 &pos);

	Model& attach_model_no_reg(Entity &e, const Path &filename);
	Model& attach_model(Entity &e, const Path &filename);
	void unattach_model(Model& m);

	MultiInstance* create_object_multi(const Path &filename, const Array<vec3> &pos, const Array<quaternion> &ang);

	void set_active_physics(Entity *o, bool active, bool passive);//, bool test_collisions);

	bool unregister(BaseClass *o);
	void delete_entity(Entity *e);
	void delete_link(Link *l);

	void add_link(Link *l);

	Path filename;
	color background;
	Array<Model*> skybox;
	Fog fog;

	Light *create_light_parallel(const quaternion &ang, const color &c);
	Light *create_light_point(const vec3 &p, const color &c, float r);
	Light *create_light_cone(const vec3 &p, const quaternion &ang, const color &c, float r, float t);

	Camera *create_camera(const vec3 &pos, const quaternion &ang);

	ParticleManager *particle_manager;

	void iterate(float dt);
	void iterate_physics(float dt);
	void iterate_animations(float dt);

	void shift_all(const vec3 &dpos);
	vec3 get_g(const vec3 &pos) const;


	float speed_of_sound;

	vec3 gravity;


	int physics_num_steps, physics_num_link_steps;

	bool net_msg_enabled;
	Array<GodNetMessage> net_messages;

	Entity *ego;


	Array<LevelData::ScriptData> scripts;


	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;

	Array<Link*> links;

	PhysicsMode physics_mode;


	base::optional<CollisionData> trace(const vec3 &p1, const vec3 &p2, int mode, Entity *o_ignore = nullptr);

	typedef void callback();
	using Callback = Callable<void()>;
	struct Observer {
		string msg;
		const Callback *f;
	};
	void subscribe(const string &msg, const Callback &f);
	void notify(const string &msg);

	Array<Observer> observers;
	struct MessageData {
		Entity *e;
		vec3 v;
	} msg_data;

	int ch_iterate = -1, ch_animation = -1;
};
extern World world;


void GodInit(int ch_iter);
void GodEnd();
bool GodLoadWorld(const Path &filename);


Light* attach_light_parallel(Entity* e, const color& c);
Light* attach_light_point(Entity* e, const color& c, float r);
Light* attach_light_cone(Entity* e, const color& c, float r, float theta);

enum {
	NET_MSG_CREATE_OBJECT = 1000,
	NET_MSG_DELETE_OBJECT = 1002,
	NET_MSG_SCTEXT = 2000
};

