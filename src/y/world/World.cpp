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

#if HAS_LIB_BULLET
#include <btBulletDynamicsCommon.h>
//#include <BulletCollision/CollisionShapes/btConvexPointCloudShape.h>
#endif

#include "Camera.h"



//#define _debug_matrices_


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


int num_insane=0;

inline bool TestVectorSanity(vec3 &v, const char *name) {
	if (inf_v(v)) {
		num_insane++;
		v=v_0;
		if (num_insane>100)
			return false;
		msg_error(format("Vektor %s unendlich!!!!!!!",name));
		return true;
	}
	return false;
}







void GodInit(int ch_iter) {
#ifdef _X_ALLOW_X_
	world.ch_iterate = profiler::create_channel("world", ch_iter);
	world.ch_animation = profiler::create_channel("animation", ch_iter);
#endif
}

void GodEnd() {
}

void send_collision(SolidBody *a, const CollisionData &col) {
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
		auto a = static_cast<SolidBody*>(obA->getUserPointer());
		auto b = static_cast<SolidBody*>(obB->getUserPointer());
		int np = contactManifold->getNumContacts();
		for (int j=0; j<np; j++) {
			auto &pt = contactManifold->getContactPoint(j);
			if (pt.getDistance() <= 0) {
				if (a->active)
					send_collision(a, {b->owner, b, bt_get_v(pt.m_positionWorldOnB), bt_get_v(pt.m_normalWorldOnB)});
				if (b->active)
					send_collision(b, {a->owner, a, bt_get_v(pt.m_positionWorldOnA), -bt_get_v(pt.m_normalWorldOnB)});
			}
		}
	}
}
#endif

World::World() {

#ifdef _X_ALLOW_X_
	particle_manager = new ParticleManager();
#endif


	physics_mode = PhysicsMode::FULL_EXTERNAL;
#if HAS_LIB_BULLET
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	overlappingPairCache = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	dynamicsWorld->setInternalTickCallback(myTickCallback);
#endif


	reset();
}

World::~World() {
#if HAS_LIB_BULLET
	delete dynamicsWorld;
	delete solver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;
#endif
}

void World::reset() {
	net_msg_enabled = false;
	net_messages.clear();

	observers.clear();

	gravity = v_0;

	for (auto *o: entities)
		delete o;
	entities.clear();



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

void add_components_no_init(Entity *ent, const Array<LevelData::ScriptData> &components) {
	for (auto &cc: components) {
		msg_write("add component " + cc.class_name);
#ifdef _X_ALLOW_X_
		auto type = PluginManager::find_class(cc.filename, cc.class_name);
		[[maybe_unused]] auto comp = ent->add_component_no_init(type, cc.var);
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
		auto o = new Entity(l.pos, quaternion::rotation(l.ang));
		auto *ll = new Light(l._color, l.radius, l.theta);
		ll->light.light.harshness = l.harshness;
		ll->light.enabled = l.enabled;
		if (ll->light.light.radius < 0)
			ll->light.allow_shadow = true;
		o->_add_component_external_no_init_(ll);
		add_components_no_init(o, l.components);
		register_entity(o);
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

		add_components_no_init(cam_main->owner, c.components);
	}
	auto& cameras = ComponentManager::get_list_family<Camera>();
	if (cameras.num == 0) {
		msg_error("no camera defined... creating one");
		cam_main = create_camera(v_0, quaternion::ID);
	}

	// objects
	ego = nullptr;
	foreachi(auto &o, ld.objects, i)
		if (!o.filename.is_empty()) {
			//try {
				auto q = quaternion::rotation(o.ang);
				auto *oo = create_object_no_reg_x(o.filename, o.name, o.pos, q);
				add_components_no_init(oo->owner, o.components);
				register_entity(oo->owner);
				if (ld.ego_index == i)
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
		auto tt = create_terrain_no_reg(t.filename, t.pos);
		register_entity(tt->owner);

		add_components_no_init(tt->owner, t.components);
		ok &= !tt->error;
	}

	// (raw) entities
	foreachi(auto &e, ld.entities, i) {
		auto ee = create_entity(e.pos, quaternion::rotation(e.ang));
		register_entity(ee);

		add_components_no_init(ee, e.components);
	}

	auto& model_list = ComponentManager::get_list_family<Model>();
	for (auto &l: ld.links) {
		Entity *a = model_list[l.object[0]]->owner;
		Entity *b = nullptr;
		if (l.object[1] >= 0)
			b = model_list[l.object[1]]->owner;
		add_link(Link::create(l.type, a, b, l.pos, quaternion::rotation(l.ang)));
	}

	systems = ld.systems;

	net_msg_enabled = true;
	return ok;
}

void World::add_link(Link *l) {
	links.add(l);
#if HAS_LIB_BULLET
	dynamicsWorld->addConstraint(l->con, true);
#endif
}


Terrain *World::create_terrain_no_reg(const Path &filename, const vec3 &pos) {

	auto o = create_entity(pos, quaternion::ID);

	auto t = o->add_component<Terrain>();
	t->load(engine.context, filename);

	[[maybe_unused]] auto col = o->add_component<TerrainCollider>();

	auto sb = o->add_component<SolidBody>();
	sb->mass = 10000.0f;
	sb->theta_0 = mat3::ZERO;
	sb->passive = true;

	return t;
}

Terrain *World::create_terrain(const Path &filename, const vec3 &pos) {
	auto t = create_terrain_no_reg(filename, pos);
	register_entity(t->owner);
	return t;
}

bool GodLoadWorld(const Path &filename) {
	LevelData level_data;
	bool ok = level_data.load(engine.map_dir | filename.with(".world"));
	ok &= world.load(level_data);
	return ok;
}

Entity *World::create_entity(const vec3 &pos, const quaternion &ang) {
	auto e = new Entity(pos, ang);
	entities.add(e);
	return e;
}

void World::register_entity(Entity *e) {
	e->on_init_rec();

#if HAS_LIB_BULLET
	if (auto sb = e->get_component<SolidBody>())
		dynamicsWorld->addRigidBody(sb->body);
#endif

	msg_data.e = e;
	notify("entity-add");
}

Model *World::create_object(const Path &filename, const vec3 &pos, const quaternion &ang) {
	auto o = create_object_no_reg_x(filename, "", pos, ang);
	register_entity(o->owner);
	return o;
}

Model *World::create_object_no_reg(const Path &filename, const vec3 &pos, const quaternion &ang) {
	return create_object_no_reg_x(filename, "", pos, ang);
}

Model *World::create_object_no_reg_x(const Path &filename, const string &name, const vec3 &pos, const quaternion &ang) {
	auto e = create_entity(pos, ang);
	auto& m = attach_model_no_reg(*e, filename);
	m.script_data.name = name;
	return &m;
}


Model& World::attach_model_no_reg(Entity &e, const Path &filename) {
	if (engine.resetting_game)
		throw Exception("create_object during game reset");

	if (filename.is_empty())
		throw Exception("create_object: empty filename");

	//msg_write(on);
	auto *m = engine.resource_manager->load_model(filename);

	e._add_component_external_no_init_(m);
	m->update_matrix();


	// automatic components
	if (m->_template->solid_body) {
		[[maybe_unused]] auto col = (MeshCollider*)e.add_component_no_init(MeshCollider::_class, "");
		[[maybe_unused]] auto sb = (SolidBody*)e.add_component_no_init(SolidBody::_class, "");
	}

	if (m->_template->skeleton)
		e.add_component_no_init(Skeleton::_class, "");

	if (m->_template->animator)
		e.add_component_no_init(Animator::_class, "");

	return *m;
}

Model& World::attach_model(Entity &e, const Path &filename) {
	auto& m = attach_model_no_reg(e, filename);
	//m.on_init();
	e.on_init_rec(); // FIXME might re-initialize too much...

#if HAS_LIB_BULLET
	if (auto sb = e.get_component<SolidBody>())
		dynamicsWorld->addRigidBody(sb->body);
#endif

	return m;
}

void World::unattach_model(Model& m) {
#if HAS_LIB_BULLET
	if (auto sb = m.owner->get_component<SolidBody>())
		dynamicsWorld->removeRigidBody(sb->body);
#endif

	m.owner->delete_component(&m);
}

MultiInstance* World::create_object_multi(const Path &filename, const Array<vec3> &pos, const Array<quaternion> &ang) {
	auto e = create_entity(vec3::ZERO, quaternion::ID);
	auto mi = (MultiInstance*)e->add_component_no_init(MultiInstance::_class, "");

	mi->model = engine.resource_manager->load_model(filename);

	for (int i=0; i<pos.num; i++)
		mi->matrices.add(mat4::translation(pos[i]) * mat4::rotation(ang[i]));

	return mi;
}


void World::set_active_physics(Entity *o, bool active, bool passive) { //, bool test_collisions) {
	auto sb = o->get_component<SolidBody>();
	auto c = o->get_component<Collider>();

#if HAS_LIB_BULLET
	btScalar mass(active ? sb->mass : 0);
	btVector3 local_inertia(0, 0, 0);
	if (c->col_shape) {
		c->col_shape->calculateLocalInertia(mass, local_inertia);
		sb->theta_0._00 = local_inertia.x();
		sb->theta_0._11 = local_inertia.y();
		sb->theta_0._22 = local_inertia.z();
	}
	sb->body->setMassProps(mass, local_inertia);
	if (passive and !sb->passive)
		dynamicsWorld->addRigidBody(sb->body);
	if (!passive and sb->passive)
		dynamicsWorld->removeRigidBody(sb->body);

	/*if (!passive and test_collisions) {
		msg_error("FIXME pure collision");
		dynamicsWorld->addCollisionObject(o->body);
	}*/
#endif


	sb->active = active;
	sb->passive = passive;
	//b->test_collisions = test_collisions;
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

void World::unregister_entity(Entity *e) {
	if (false)
		AddNetMsg(NET_MSG_DELETE_OBJECT, e->object_id, "");

	// ego...
	if (e == ego)
		ego = nullptr;


#if HAS_LIB_BULLET
	if (auto sb = e->get_component<SolidBody>())
		dynamicsWorld->removeRigidBody(sb->body);
#endif
}

void World::delete_entity(Entity *e) {
	int index = entities.find(e);
	if (index < 0)
		return;

	unregister_entity(e);
	e->on_delete_rec();

	msg_data.e = e;
	notify("entity-delete");
	entities.erase(index);

	delete e;
}

void World::delete_link(Link *l) {
	if (unregister(l))
		delete l;
}

bool World::unregister(BaseClass* x) {
	//msg_error("World.unregister  " + i2s((int)x->type));
	if (x->type == BaseClass::Type::ENTITY) {
		auto e = (Entity*)x;
		unregister_entity(e);
	} else if (x->type == BaseClass::Type::LINK) {
		foreachi(auto *l, links, i)
			if (l == x) {
				//msg_write(" -> LINK");
				links.erase(i);
				return true;
			}
	}
	return false;
}

void World::iterate_physics(float dt) {
	auto& list = ComponentManager::get_list_family<SolidBody>();

	if (physics_mode == PhysicsMode::FULL_EXTERNAL) {
#if HAS_LIB_BULLET
		dynamicsWorld->setGravity(bt_set_v(gravity));
		dynamicsWorld->stepSimulation(dt, 10);

		for (auto *o: list)
			o->get_state_from_bullet();
#endif
	} else if (physics_mode == PhysicsMode::SIMPLE) {
		for (auto *o: list)
			o->do_simple_physics(dt);
	}

	for (auto *sb: list) {
		if (auto m = sb->owner->get_component<Model>())
			m->update_matrix();
	}
}

void World::iterate_animations(float dt) {
#ifdef _X_ALLOW_X_
	profiler::begin(ch_animation);
	auto& list = ComponentManager::get_list_family<Animator>();
	for (auto *o: list)
		o->do_animation(dt);


	// TODO
	auto& list2 = ComponentManager::get_list_family<Skeleton>();
	for (auto *o: list2) {
		for (auto &b: o->bones) {
			if ([[maybe_unused]] auto *mm = b.get_component<Model>()) {
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
		iterate_physics(dt);
	} else {
		/*for (auto *o: objects)
			if (o)
				if (auto m = o->get_component<Model>())
					m->update_matrix();*/
	}

	profiler::end(ch_iterate);
#endif
}

Light* attach_light_parallel(Entity* e, const color& c) {
	auto l = e->add_component<Light>();
	l->light.light.col = c;
	return l;
}

Light* attach_light_point(Entity* e, const color& c, float r) {
	auto l = e->add_component<Light>();
	l->light.light.col = c;
	l->light.light.radius = r;
	return l;
}

Light* attach_light_cone(Entity* e, const color& c, float r, float theta) {
	auto l = e->add_component<Light>();
	l->light.light.col = c;
	l->light.light.radius = r;
	l->light.light.theta = theta;
	return l;
}

Light *World::create_light_parallel(const quaternion &ang, const color &c) {
	auto o = create_entity(v_0, ang);
	auto l = attach_light_parallel(o, c);
	register_entity(o);
	return l;
}

Light *World::create_light_point(const vec3 &pos, const color &c, float r) {
	auto o = create_entity(pos, quaternion::ID);
	auto l = attach_light_point(o, c, r);
	register_entity(o);
	return l;
}

Light *World::create_light_cone(const vec3 &pos, const quaternion &ang, const color &c, float r, float t) {
	auto o = create_entity(pos, ang);
	auto l = attach_light_cone(o, c, r, t);
	register_entity(o);
	return l;
}

Camera *World::create_camera(const vec3 &pos, const quaternion &ang) {
	auto o = create_entity(pos, ang);

	auto c = new Camera();
	o->_add_component_external_no_init_(c);
	register_entity(o);
	return c;
}


void World::shift_all(const vec3 &dpos) {
	for (auto *e: entities) {
		e->pos += dpos;
		//if (auto m = e->get_component<Model>())
		//	m->update_matrix();
	}

	for (auto &sb: ComponentManager::get_list_family<SolidBody>())
		sb->state_to_bullet();

	for (auto *m: ComponentManager::get_list_family<Model>())
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
#if HAS_LIB_BULLET
		btCollisionWorld::ClosestRayResultCallback ray_callback(bt_set_v(p1), bt_set_v(p2));
		//ray_callback.m_collisionFilterMask = FILTER_CAMERA;

		// Perform raycast
		this->dynamicsWorld->getCollisionWorld()->rayTest(bt_set_v(p1), bt_set_v(p2), ray_callback);
		if (ray_callback.hasHit()) {
			CollisionData d;
			auto sb = static_cast<SolidBody *>(ray_callback.m_collisionObject->getUserPointer());
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
	} else if (mode & TraceMode::VISIBLE) {

	}
	return base::None;
}

