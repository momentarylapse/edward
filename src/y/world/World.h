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
struct ModelRef;
namespace yrenderer {
	struct Material;
}
class Terrain;
struct TerrainRef;
struct RigidBody;
struct MultiInstance;
namespace ecs {
	struct Entity;
	class EntityManager;
}
struct Light;
struct Camera;
struct Link;
struct LevelData;
enum class LinkType;





struct CollisionData {
	ecs::Entity *entity;
	RigidBody *body;
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

	ecs::Entity* create_entity(const vec3 &pos, const quaternion &ang);
	owned<ecs::EntityManager> entity_manager;

	ecs::Entity* create_from_template(const Path &filename, const vec3 &pos, const quaternion &ang);

	Model* create_object(const Path &filename, const vec3 &pos, const quaternion &ang);
	Model* create_object_x(const Path &filename, const string &name, const vec3 &pos, const quaternion &ang);
	TerrainRef* create_terrain(const Path &filename, const vec3 &pos);

	ModelRef* attach_model(ecs::Entity* e, const Path &filename);

	MultiInstance* create_object_multi(const Path &filename, const Array<vec3> &pos, const Array<quaternion> &ang);

	void delete_entity(ecs::Entity* e);

	ecs::Entity* get_entity(int index);

	color background;
	owned_array<ModelRef> skybox;
	Fog fog;

	Light* attach_light_parallel(ecs::Entity* e, const color& c);
	Light* attach_light_point(ecs::Entity* e, const color& c, float r);
	Light* attach_light_cone(ecs::Entity* e, const color& c, float r, float theta);

	Light* create_light_parallel(const quaternion& ang, const color& c);
	Light* create_light_point(const vec3& p, const color& c, float r);
	Light* create_light_cone(const vec3& p, const quaternion& ang, const color& c, float r, float t);

	Camera* create_camera(const vec3& pos, const quaternion& ang);

	void shift_all(const vec3& dpos);

	ecs::Entity* ego();


	base::optional<CollisionData> trace(const vec3& p1, const vec3& p2, int mode, ecs::Entity* o_ignore = nullptr);

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
		ecs::Entity* e;
		vec3 v;
	} msg_data;
};
extern World world;



enum {
	NET_MSG_CREATE_OBJECT = 1000,
	NET_MSG_DELETE_OBJECT = 1002,
	NET_MSG_SCTEXT = 2000
};

