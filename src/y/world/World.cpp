/*----------------------------------------------------------------------------*\
| God                                                                          |
| -> manages objetcs and interactions                                          |
| -> loads and stores the world data (level)                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2009.11.22 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/

#include <lib/base/base.h>
#include <lib/base/sort.h>
#include <lib/config.h>
#include <lib/os/msg.h>
#include <lib/nix/nix.h>
#include <lib/kaba/kaba.h>
#include <lib/os/filesystem.h>
#include <EngineData.h>
#include <ecs/Component.h>
#include <ecs/ComponentManager.h>
#include <ecs/Entity.h>
#include <ecs/EntityManager.h>
#include "ModelManager.h"
#include "../helper/ResourceManager.h"
#include "Model.h"
#include "Terrain.h"
#include "World.h"

#include "components/RigidBody.h"
#include "components/Collider.h"
#include "components/MultiInstance.h"
#include "components/Light.h"
#include "components/Camera.h"
#include "systems/Physics.h"
#include "ecs/SystemManager.h"

#include "../plugins/PluginManager.h"




// game data
World world;


#ifdef _X_ALLOW_X_
void DrawSplashScreen(const string &str, float per);
#else
void DrawSplashScreen(const string &str, float per){}
#endif




World::World() {
	entity_manager = new ecs::EntityManager;
#ifdef _X_ALLOW_X_
	entity_manager->component_manager->f_create = [] (const kaba::Class* type) {
		return (ecs::Component*)PluginManager::create_instance(type, Array<ecs::InstanceDataVariable>{});
	};
	entity_manager->component_manager->f_apply = [this] (const kaba::Class* type, ecs::Component* c, const Array<ecs::InstanceDataVariable>& vars) {
		PluginManager::assign_variables(c, type, vars);
	};
#endif

	reset();
}

World::~World() = default;

void World::reset() {
	observers.clear();

	entity_manager->reset();

	skybox.clear();
	

	// initial data for empty world...
	fog._color = White;
	fog.mode = 0;//FOG_EXP;
	fog.distance = 10000;
	fog.enabled = false;
	fog.start = 0;
	fog.end = 100000;
}

void World::save(const Path &filename) {
	LevelData l;
	l.save(filename);
}

void World::load_soon(const Path &filename) {
	next_filename = filename;
}

Array<ecs::InstanceData> sort_components(const Array<ecs::InstanceData>& components) {
	auto r = components;
	for (int i=0; i<r.num; i++)
		for (int k=i+1; k<r.num; k++) {
			if (!r[i].filename.is_empty() and r[k].filename.is_empty())
				r.swap(i, k);
			else if (r[i].class_name == "RigidBody" and r[k].class_name.match("*Collider"))
				r.swap(i, k);
		}
	return r;
}

void add_user_components(ecs::EntityManager* em, ecs::Entity *ent, const Array<ecs::InstanceData>& components) {
	for (auto &cc: sort_components(components)) {
		msg_write("add component " + cc.class_name);
		auto type = PluginManager::find_class(cc.filename, cc.class_name);
		[[maybe_unused]] auto comp = em->_add_component_generic_(ent, type, cc.variables);

		// templates -> recursion...
		if (type == TemplateRef::_class)
			if (auto t = ((TemplateRef*)comp)->_template)
				add_user_components(em, ent, t->components);
	}
}

bool World::load(const LevelData& ld) {
	bool ok = true;
	reset();

	if (auto physics = ecs::SystemManager::get<Physics>()) {
		physics->mode = PhysicsMode::FULL_EXTERNAL;
		physics->enabled = ld.physics_enabled;
		physics->mode = ld.physics_mode;
		physics->gravity = ld.gravity;
		physics->collisions_enabled = true;//LevelData.physics_enabled;
	}

	fog = ld.fog;

	// skybox
	skybox.resize(ld.skybox_filename.num);
	for (int i=0; i<skybox.num; i++) {
		skybox[i] = new ModelRef;
		skybox[i]->owner = new ecs::Entity(v_0, quaternion::rotation_v(ld.skybox_ang[i])); // FIXME data leak... eh
		skybox[i]->model = engine.resource_manager->load_model(ld.skybox_filename[i]);
	}
	background = ld.background_color;


	// entities
	for (const auto& e: ld.entities) {
		auto ee = create_entity(e.pos, e.ang);
		add_user_components(entity_manager.get(), ee, e.components);
	}

	// pick main camera
	auto& cameras = entity_manager->get_component_list<Camera>();
	if (cameras.num == 0) {
		msg_error("no camera defined... creating one");
		cam_main = create_camera(v_0, quaternion::ID);
	} else if (!cam_main) {
		cam_main = cameras[0];
	}

	return ok;
}

ecs::Entity* World::ego() {
	auto& list = entity_manager->get_component_list<EgoMarker>();
	if (list.num >= 1)
		return list[0]->owner;
	return nullptr;
}

ecs::Entity* World::get_entity(int index) {
	if (index < 0 or index >= entity_manager->entities.num)
		return nullptr;
	return entity_manager->entities[index];
}


TerrainRef* World::create_terrain(const Path &filename, const vec3 &pos) {
	auto e = create_entity(pos, quaternion::ID);

	auto t = entity_manager->add_component<TerrainRef>(e);
	t->terrain = engine.resource_manager->load_terrain_lazy(filename);
	t->terrain->reload(engine.resource_manager);
	t->material = engine.resource_manager->load_material("");

	entity_manager->add_component<TerrainCollider>(e);
	entity_manager->add_component<RigidBody>(e, {{"dynamic", false}});

	return t;
}

ecs::Entity *World::create_entity(const vec3& pos, const quaternion& ang) {
	return entity_manager->create_entity(pos, ang);
}

ecs::Entity* World::create_from_template(const Path& filename, const vec3 &pos, const quaternion& ang) {
	auto e = create_entity(pos, ang);

	if (const auto t = engine.resource_manager->load_template(filename)) {
		add_user_components(entity_manager.get(), e, t->components);
	} else if (os::fs::exists(engine.object_dir | filename.with(".model"))) {
		attach_model(e, filename);
	}

	return e;
}

Model *World::create_object(const Path &filename, const vec3 &pos, const quaternion &ang) {
	return create_object_x(filename, "", pos, ang);
}

Model *World::create_object_x(const Path &filename, const string &name, const vec3 &pos, const quaternion &ang) {
	if (engine.resetting_game)
		throw Exception("create_object during game reset");

	if (filename.is_empty())
		throw Exception("create_object: empty filename");

	auto e = create_entity(pos, ang);
	auto m = attach_model(e, filename);

	if (name != "" and false) {
		auto tag = entity_manager->add_component<NameTag>(e);
		tag->name = name;
	}

	return m->model;
}


ModelRef* World::attach_model(ecs::Entity* e, const Path& filename) {
	auto mr = entity_manager->add_component<ModelRef>(e);
	mr->model = engine.resource_manager->load_model(filename);

	// automatic components
	add_user_components(entity_manager.get(), e, mr->model->_template->components);

	return mr;
}

MultiInstance* World::create_object_multi(const Path &filename, const Array<vec3> &pos, const Array<quaternion> &ang) {
	auto e = create_entity(vec3::ZERO, quaternion::ID);
	auto mi = entity_manager->add_component<MultiInstance>(e);

	mi->model = engine.resource_manager->load_model_copy(filename);

	for (int i=0; i<pos.num; i++)
		mi->matrices.add(mat4::translation(pos[i]) * mat4::rotation(ang[i]));

	return mi;
}

void World::subscribe(const string &msg, const Callback &f) {
	observers.add({msg, &f});
}

void World::notify(const string &msg) {
	for (auto &o: observers)
		if (o.msg == msg) {
			(*o.f)();
		}
}

void World::delete_entity(ecs::Entity *e) {

	msg_data.e = e;
	notify("entity-delete");
	entity_manager->delete_entity(e);
}

Light* World::attach_light_parallel(ecs::Entity* e, const color& c) {
	auto l = entity_manager->add_component<Light>(e);
	l->light.type = yrenderer::LightType::DIRECTIONAL;
	l->light.col = c;
	return l;
}

// r is deprecated... c used to defined power -> implicit radius
Light* World::attach_light_point(ecs::Entity* e, const color& c, float r) {
	auto l = entity_manager->add_component<Light>(e);
	l->light.type = yrenderer::LightType::POINT;
	l->light.col = c;
	//l->light.power = yrenderer::Light::_radius_to_power(r);
	return l;
}

Light* World::attach_light_cone(ecs::Entity* e, const color& c, float r, float theta) {
	auto l = entity_manager->add_component<Light>(e);
	l->light.type = yrenderer::LightType::CONE;
	l->light.col = c;
	//l->light.power = yrenderer::Light::_radius_to_power(r);
	l->light.theta = theta;
	return l;
}

Light *World::create_light_parallel(const quaternion &ang, const color &c) {
	auto e = create_entity(v_0, ang);
	return attach_light_parallel(e, c);
}

Light *World::create_light_point(const vec3 &pos, const color &c, float r) {
	auto e = create_entity(pos, quaternion::ID);
	return attach_light_point(e, c, r);
}

Light *World::create_light_cone(const vec3 &pos, const quaternion &ang, const color &c, float r, float t) {
	auto e = create_entity(pos, ang);
	return attach_light_cone(e, c, r, t);
}

Camera *World::create_camera(const vec3 &pos, const quaternion &ang) {
	auto e = create_entity(pos, ang);
	return entity_manager->add_component<Camera>(e);
}


void World::shift_all(const vec3 &dpos) {
	entity_manager->shift_all(dpos);

	if (auto physics = ecs::SystemManager::get<Physics>())
		physics->update_all_bullet();

	msg_data.v = dpos;
	notify("shift");
}

enum TraceMode {
	PHYSICAL = 1,
	VISIBLE = 2,
	SIMPLE = 4
};

base::optional<CollisionData> World::trace(const vec3 &p1, const vec3 &p2, int mode, ecs::Entity *o_ignore) {
	if (mode & TraceMode::PHYSICAL) {
		if (auto physics = ecs::SystemManager::get<Physics>())
			return physics->trace(p1, p2, mode, o_ignore);
	} else if (mode & TraceMode::VISIBLE) {

	}
	return base::None;
}

