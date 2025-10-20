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


class Physics;
class Model;
class Object;
namespace yrenderer {
	class Material;
}
class Terrain;
class SolidBody;
class MultiInstance;
class Entity;
class EntityManager;
class Light;
class Camera;
class ParticleManager;
class LegacyParticle;
class Link;
struct LevelData;
enum class LinkType;





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
	owned<EntityManager> entity_manager;

	Entity* load_template(const Path &filename, const vec3 &pos, const quaternion &ang);

	Model* create_object(const Path &filename, const vec3 &pos, const quaternion &ang);
	Model* create_object_x(const Path &filename, const string &name, const vec3 &pos, const quaternion &ang);
	Terrain* create_terrain(const Path &filename, const vec3 &pos);

	Model* attach_model(Entity* e, const Path &filename);
	void unattach_model(Model* m);

	MultiInstance* create_object_multi(const Path &filename, const Array<vec3> &pos, const Array<quaternion> &ang);

	void delete_entity(Entity *e);

	color background;
	Array<Model*> skybox;
	Fog fog;

	Light* attach_light_parallel(Entity* e, const color& c);
	Light* attach_light_point(Entity* e, const color& c, float r);
	Light* attach_light_cone(Entity* e, const color& c, float r, float theta);

	Light *create_light_parallel(const quaternion &ang, const color &c);
	Light *create_light_point(const vec3 &p, const color &c, float r);
	Light *create_light_cone(const vec3 &p, const quaternion &ang, const color &c, float r, float t);

	Camera *create_camera(const vec3 &pos, const quaternion &ang);

	ParticleManager *particle_manager;

	void iterate(float dt);
	void iterate_animations(float dt);

	void shift_all(const vec3 &dpos);

	bool net_msg_enabled;
	Array<GodNetMessage> net_messages;

	Entity* ego();


	Array<ScriptInstanceData> systems;

	Physics* physics;


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


enum {
	NET_MSG_CREATE_OBJECT = 1000,
	NET_MSG_DELETE_OBJECT = 1002,
	NET_MSG_SCTEXT = 2000
};

