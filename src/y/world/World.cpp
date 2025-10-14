/*----------------------------------------------------------------------------*\
| God                                                                          |
| -> manages objetcs and interactions                                          |
| -> loads and stores the world data (level)                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2009.11.22 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/

#include <algorithm>
#include <lib/config.h>
#include <lib/os/msg.h>
#include <lib/nix/nix.h>
#include <lib/kaba/kaba.h>
#include "../y/EngineData.h"
#include "../y/Component.h"
#include "../y/ComponentManager.h"
#include "../y/Entity.h"
#include "../y/EntityManager.h"
#include "../meta.h"
#include "ModelManager.h"
#include "../helper/ResourceManager.h"
#include "Link.h"
#include "Light.h"
#include <lib/yrenderer/Material.h>
#include "Model.h"
#include "Terrain.h"
#include "World.h"

#include "components/SolidBody.h"
#include "components/Collider.h"
#include "components/Animator.h"
#include "components/Skeleton.h"
#include "components/MultiInstance.h"

#ifdef _X_ALLOW_X_
#include "../fx/ParticleManager.h"
#include "../plugins/PluginManager.h"
#include <lib/profiler/Profiler.h>
#endif


#include "Camera.h"
#include "PhysicsSimulation.h"
#include "lib/base/sort.h"


//#define _debug_matrices_



// game data
World world;


#ifdef _X_ALLOW_X_
void DrawSplashScreen(const string &str, float per);
#else
void DrawSplashScreen(const string &str, float per){}
#endif


// network messages
void AddNetMsg(int msg, int argi0, const string &args)
{
#if 0
#ifdef _X_ALLOW_X_
	if ((!world.net_msg_enabled) || (!Net.Enabled))
		return;
	GodNetMessage m;
	m.msg = msg;
	m.arg_i[0] = argi0;
	m.arg_s = args;
	world.net_messages.add(m);
#endif
#endif
}







void GodInit(int ch_iter) {
	// FIXME: heap allocate world?
#ifdef _X_ALLOW_X_
	world.ch_iterate = profiler::create_channel("world", ch_iter);
	world.ch_animation = profiler::create_channel("animation", ch_iter);
#endif
}

void GodEnd() {
}

World::World() {
	entity_manager = new EntityManager;
#ifdef _X_ALLOW_X_
	entity_manager->component_manager->f_create = [] (const kaba::Class* type) {
		return (Component*)PluginManager::create_instance(type, Array<ScriptInstanceDataVariable>{});
	};
	entity_manager->component_manager->f_apply = [this] (const kaba::Class* type, Component* c, const Array<ScriptInstanceDataVariable>& vars) {
		/*Array<ScriptInstanceDataVariable> vars;
		for (const auto& [k, v]: params)
			vars.add({k, v.str()});*/
		PluginManager::assign_variables(c, type, vars);
		if (type == ModelRef::_class) {
			auto cc = static_cast<ModelRef*>(c);
			cc->model = engine.resource_manager->load_model(cc->filename);
		}
		if (type == TerrainRef::_class) {
			auto cc = static_cast<TerrainRef*>(c);
			cc->terrain = new Terrain(engine.context, cc->filename);
		}
	};
#endif

#ifdef _X_ALLOW_X_
	particle_manager = new ParticleManager(entity_manager.get());
#endif


	physics_mode = PhysicsMode::FULL_EXTERNAL;
	physics_simulation = new PhysicsSimulation(this);


	reset();
}

World::~World() {
	delete physics_simulation;
}

void World::reset() {
	net_msg_enabled = false;
	net_messages.clear();

	observers.clear();

	gravity = v_0;

	entity_manager->reset();



	// skybox
	//   (models deleted by meta)
	skybox.clear();
	

	// initial data for empty world...
	fog._color = White;
	fog.mode = 0;//FOG_EXP;
	fog.distance = 10000;
	fog.enabled = false;
	fog.start = 0;
	fog.end = 100000;
	speed_of_sound = 1000;

	physics_mode = PhysicsMode::FULL_EXTERNAL;
	engine.physics_enabled = false;
	engine.collisions_enabled = true;
	physics_num_steps = 10;
	physics_num_link_steps = 5;


	// physics
#ifdef _X_ALLOW_X_
	//LinksReset();
#endif
}

void World::save(const Path &filename) {
	LevelData l;
	l.save(filename);
}

void World::load_soon(const Path &filename) {
	next_filename = filename;
}

Array<ScriptInstanceData> sort_components(const Array<ScriptInstanceData>& components) {
	auto r = components;
	for (int i=0; i<r.num; i++)
		for (int k=i+1; k<r.num; k++) {
			if (!r[i].filename.is_empty() and r[k].filename.is_empty())
				r.swap(i, k);
			else if (r[i].class_name == "SolidBody" and r[k].class_name == "MeshCollider")
				r.swap(i, k);
			else if (r[i].class_name == "SolidBody" and r[k].class_name == "TerrainCollider")
				r.swap(i, k);
		}
	return r;
}

void add_user_components(EntityManager* em, Entity *ent, const Array<ScriptInstanceData>& components) {
	for (auto &cc: sort_components(components)) {
		msg_write("add component " + cc.class_name);
#ifdef _X_ALLOW_X_
		auto type = PluginManager::find_class(cc.filename, cc.class_name);
		/*base::map<string, Any> params;
		for (const auto& v: cc.variables)
			params.set(v.name, v.value);*/
		[[maybe_unused]] auto comp = em->_add_component_generic_(ent, type, cc.variables);
#endif
	}
}

bool World::load(const LevelData &ld) {
	net_msg_enabled = false;
	bool ok = true;
	reset();


	engine.physics_enabled = ld.physics_enabled;
	world.physics_mode = ld.physics_mode;
	engine.collisions_enabled = true;//LevelData.physics_enabled;
	gravity = ld.gravity;
	fog = ld.fog;

	for (auto &l: ld.lights) {
		auto o = create_entity(l.pos, quaternion::rotation(l.ang));
		auto ll = entity_manager->add_component<Light>(o);
		ll->light.init(l.type, l._color, l.theta);
		ll->light.harshness = l.harshness;
		ll->light.enabled = l.enabled;
		if (ll->light.type == yrenderer::LightType::DIRECTIONAL)
			ll->light.allow_shadow = true;
		else
			ll->light.power = yrenderer::Light::_radius_to_power(l.radius);

		add_user_components(entity_manager.get(), o, l.components);
	}

	// skybox
	skybox.resize(ld.skybox_filename.num);
	for (int i=0; i<skybox.num; i++) {
		skybox[i] = engine.resource_manager->load_model(ld.skybox_filename[i]);
		if (skybox[i])
			skybox[i]->owner = new Entity(v_0, quaternion::rotation_v(ld.skybox_ang[i]));
	}
	background = ld.background_color;

	for (auto &c: ld.cameras) {
		auto cc = create_camera(c.pos, quaternion::rotation(c.ang));
		cam_main = cc;
		cc->min_depth = c.min_depth;
		cc->max_depth = c.max_depth;
		cc->exposure = c.exposure;
		cc->bloom_factor = c.bloom_factor;
		cc->fov = c.fov;

		add_user_components(entity_manager.get(), cc->owner, c.components);
	}

	// objects
	ego = nullptr;
	foreachi(auto &o, ld.objects, i)
		if (!o.filename.is_empty()) {
			//try {
				auto q = quaternion::rotation(o.ang);
				auto *oo = create_object_x(o.filename, o.name, o.pos, q);

				add_user_components(entity_manager.get(), oo->owner, o.components);
				if (ld.ego_index == i + 1000000000)
					ego = oo->owner;
				if (i % 5 == 0)
					DrawSplashScreen("Objects", (float)i / (float)ld.objects.num / 5 * 3);

			/*} catch (...) {
				ok = false;
			}*/
		}

	// terrains
	foreachi(auto &t, ld.terrains, i) {
		DrawSplashScreen("Terrain...", 0.6f + (float)i / (float)ld.terrains.num * 0.4f);
		auto tt = create_terrain(t.filename, t.pos);

		add_user_components(entity_manager.get(), tt->owner, t.components);
		ok &= !tt->error;
	}

	// (raw) entities
	foreachi(auto &e, ld.entities, i) {
		auto ee = create_entity(e.pos, e.ang);

		add_user_components(entity_manager.get(), ee, e.components);
		if (ld.ego_index == i)
			ego = ee;
	}

	auto& model_list = entity_manager->get_component_list<Model>();
	for (auto &l: ld.links) {
		Entity *a = model_list[l.object[0]]->owner;
		Entity *b = nullptr;
		if (l.object[1] >= 0)
			b = model_list[l.object[1]]->owner;
		add_link(Link::create(l.type, a, b, l.pos, quaternion::rotation(l.ang)));
	}

	auto& cameras = entity_manager->get_component_list<Camera>();
	if (cameras.num == 0) {
		msg_error("no camera defined... creating one");
		cam_main = create_camera(v_0, quaternion::ID);
	} else if (!cam_main) {
		cam_main = cameras[0];
	}

	systems = ld.systems;

	net_msg_enabled = true;
	return ok;
}

void World::add_link(Link *l) {
	links.add(l);
	physics_simulation->add_link(l);
}


Terrain *World::create_terrain(const Path &filename, const vec3 &pos) {
	auto e = create_entity(pos, quaternion::ID);

	auto t = entity_manager->add_component<Terrain>(e);
	t->load(engine.context, filename);

	entity_manager->add_component<TerrainCollider>(e);
	entity_manager->add_component<SolidBody>(e, {{"physics_active", "", "false"}});

	return t;
}

bool GodLoadWorld(const Path& filename) {
	LevelData level_data;
	bool ok = level_data.load(filename);
	ok &= world.load(level_data);
	return ok;
}

Entity *World::create_entity(const vec3& pos, const quaternion& ang) {
	return entity_manager->create_entity(pos, ang);
}

Entity* World::load_template(const Path& filename, const vec3 &pos, const quaternion& ang) {
	auto e = create_entity(pos, ang);

	const auto t = LevelData::load_template(engine.object_dir | filename);
	add_user_components(entity_manager.get(), e, t.components);

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

	return m;
}


Model* World::attach_model(Entity* e, const Path& filename) {
	auto *m = engine.resource_manager->load_model(filename);

	entity_manager->_add_component_external_(e, m);
	m->update_matrix();

	// automatic components
	add_user_components(entity_manager.get(), e, m->_template->components);

	return m;
}

void World::unattach_model(Model* m) {
	entity_manager->delete_component(m->owner, m);
}

MultiInstance* World::create_object_multi(const Path &filename, const Array<vec3> &pos, const Array<quaternion> &ang) {
	auto e = create_entity(vec3::ZERO, quaternion::ID);
	auto mi = entity_manager->add_component<MultiInstance>(e);

	mi->model = engine.resource_manager->load_model(filename);

	for (int i=0; i<pos.num; i++)
		mi->matrices.add(mat4::translation(pos[i]) * mat4::rotation(ang[i]));

	return mi;
}


void World::set_active_physics(Entity *o, bool active, bool passive) { //, bool test_collisions) {
	physics_simulation->set_active_physics(o, active, passive);
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

void World::delete_entity(Entity *e) {
	//e->on_delete_rec();

	msg_data.e = e;
	notify("entity-delete");
	entity_manager->delete_entity(e);
}

void World::delete_link(Link* _l) {
	foreachi(auto *l, links, i)
		if (l == _l) {
			//msg_write(" -> LINK");
			links.erase(i);
			delete l;
		}
}

void World::iterate_animations(float dt) {
#ifdef _X_ALLOW_X_
	profiler::begin(ch_animation);
	auto& list = entity_manager->get_component_list<Animator>();
	for (auto *o: list)
		o->do_animation(dt);


	// TODO
	auto& list2 = entity_manager->get_component_list<Skeleton>();
	for (auto o: list2) {
		for (auto b: o->bones) {
			if ([[maybe_unused]] auto *mm = b->get_component<Model>()) {
//				b.dmatrix = matrix::translation(b.cur_pos) * matrix::rotation(b.cur_ang);
//				mm->_matrix = o->get_owner<Entity3D>()->get_matrix() * b.dmatrix;
			}
		}
	}
		//o->do_animation(dt);
	profiler::end(ch_animation);
#endif
}

void World::iterate(float dt) {
	if (dt == 0)
		return;
#ifdef _X_ALLOW_X_
	profiler::begin(ch_iterate);
	if (engine.physics_enabled) {
		physics_simulation->on_iterate(dt);
	} else {
		/*for (auto *o: objects)
			if (o)
				if (auto m = o->get_component<Model>())
					m->update_matrix();*/
	}

	profiler::end(ch_iterate);
#endif
}

Light* World::attach_light_parallel(Entity* e, const color& c) {
	auto l = entity_manager->add_component<Light>(e);
	l->light.type = yrenderer::LightType::DIRECTIONAL;
	l->light.col = c;
	return l;
}

// r is deprecated... c used to defined power -> implicit radius
Light* World::attach_light_point(Entity* e, const color& c, float r) {
	auto l = entity_manager->add_component<Light>(e);
	l->light.type = yrenderer::LightType::POINT;
	l->light.col = c;
	//l->light.power = yrenderer::Light::_radius_to_power(r);
	return l;
}

Light* World::attach_light_cone(Entity* e, const color& c, float r, float theta) {
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

	physics_simulation->update_all_bullet();

	for (auto *m: entity_manager->get_component_list<Model>())
		m->update_matrix();

	msg_data.v = dpos;
	notify("shift");
}

vec3 World::get_g(const vec3 &pos) const {
	return gravity;
}

enum TraceMode {
	PHYSICAL = 1,
	VISIBLE = 2,
	SIMPLE = 4
};

base::optional<CollisionData> World::trace(const vec3 &p1, const vec3 &p2, int mode, Entity *o_ignore) {
	if (mode & TraceMode::PHYSICAL) {
		return physics_simulation->trace(p1, p2, mode, o_ignore);
	} else if (mode & TraceMode::VISIBLE) {

	}
	return base::None;
}

