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


#include "../lib/base/base.h"
#include "../lib/base/callable.h"
#include "../lib/os/path.h"
#include "../lib/image/color.h"
#include "LevelData.h"


class Model;
class Object;
class Material;
class Terrain;
class SolidBody;
class Entity;
class TemplateDataScriptVariable;
class Light;
class ParticleManager;
class Particle;
class Link;
class LevelData;
enum class LinkType;
namespace audio {
	class Sound;
}


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


struct PartialModel {
	Model *model;
	Material *material;
	int mat_index;
	float d;
	bool shadow, transparent;
	void clear();
};

struct PartialModelMulti {
	Model *model;
	Material *material;
	Array<mat4> matrices;
	int mat_index;
	float d;
	bool shadow, transparent;
	void clear();
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

	Entity *create_object(const Path &filename, const vec3 &pos, const quaternion &ang);
	Entity *create_object_no_reg(const Path &filename, const vec3 &pos, const quaternion &ang);
	Entity *create_object_no_reg_x(const Path &filename, const string &name, const vec3 &pos, const quaternion &ang);
	Terrain *create_terrain(const Path &filename, const vec3 &pos);
	Terrain *create_terrain_no_reg(const Path &filename, const vec3 &pos);

	Object* create_object_multi(const Path &filename, const Array<vec3> &pos, const Array<quaternion> &ang);

	int next_object_index = -1;
	void request_next_object_index(int i);

	void register_object(Entity *o);
	void unregister_object(Entity *o);
	void set_active_physics(Entity *o, bool active, bool passive);//, bool test_collisions);

	bool unregister(BaseClass *o);

	void register_model(Model *m);
	void unregister_model(Model *m);

	void register_model_multi(Model *m, const Array<mat4> &matrices);

	void add_link(Link *l);

	Path filename;
	color background;
	Array<Model*> skybox;
	Fog fog;

	Array<Light*> lights;
	Light *add_light_parallel(const quaternion &ang, const color &c);
	Light *add_light_point(const vec3 &p, const color &c, float r);
	Light *add_light_cone(const vec3 &p, const quaternion &ang, const color &c, float r, float t);

	ParticleManager *particle_manager;
	void add_particle(Particle *p);

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

	// content of the world
	Array<Entity*> _objects;
	Entity *ego;
	int num_reserved_objects;

	Array<PartialModel> sorted_opaque, sorted_trans;
	Array<PartialModelMulti> sorted_multi;


	Array<LevelData::ScriptData> scripts;


	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;

	Array<Link*> links;

	PhysicsMode physics_mode;


	bool _cdecl trace(const vec3 &p1, const vec3 &p2, CollisionData &d, bool simple_test, Entity *o_ignore = nullptr);

	Array<audio::Sound*> sounds;
	void add_sound(audio::Sound *s);

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


enum {
	NET_MSG_CREATE_OBJECT = 1000,
	NET_MSG_DELETE_OBJECT = 1002,
	NET_MSG_SCTEXT = 2000
};

