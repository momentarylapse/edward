/*
 * DataWorld.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "DataWorld.h"

#include <sys/stat.h>

#include "WorldObject.h"
#include "WorldTerrain.h"
#include "WorldLink.h"
#include "lib/any/conversion.h"
#include "lib/kaba/syntax/Class.h"
#include "world/Camera.h"
#include "world/components/Animator.h"
#include "world/components/Collider.h"
#include "world/components/Skeleton.h"
#include "world/components/SolidBody.h"
#if 0 //HAS_LIB_GL
#include "../../mode/world/ModeWorld.h"
#endif
#include "../../Session.h"
#include "../../storage/Storage.h"
#include <y/world/Model.h>
#include <y/world/Terrain.h>
#include <y/world/World.h>
#include <y/world/Light.h>
#include <y/world/Camera.h>
#include <y/meta.h>
#include "../action/entity/ActionWorldEditComponent.h"
#include "../action/object/ActionWorldAddObject.h"
#include "../action/terrain/ActionWorldAddTerrain.h"
#include "../action/ActionWorldPaste.h"
#include <lib/os/msg.h>
#include <lib/base/iter.h>
#include <mode_world/action/entity/ActionWorldAddEntity.h>
#include <mode_world/action/terrain/ActionWorldEditTerrainMetaData.h>

#include "../action/ActionWorldDeleteSelection.h"
#include <lib/ygraphics/graphics-impl.h>
#include <y/helper/ResourceManager.h>
#include <y/EntityManager.h>

const kaba::Class* EdwardTag::_class = nullptr;

ScriptInstanceData& WorldEntity::get(const string& class_name) {
	for (auto& c: components)
		if (c.class_name == class_name)
			return c;
	static ScriptInstanceData dummy;
	dummy.class_name = "";
	return dummy;
}

ScriptInstanceData& EdwardTag::get(const string& class_name) {
	for (auto& c: unknown_components)
		if (c.class_name == class_name)
			return c;
	static ScriptInstanceData dummy;
	dummy.class_name = "";
	return dummy;
}


DataWorld::DataWorld(Session *s) :
	Data(s, FD_WORLD)
{
	entity_manager = new EntityManager;
	entity_manager->component_manager->f_create = [] (const kaba::Class* type) -> Component* {
		if (type == Camera::_class)
			return new Camera();
		if (type == Light::_class)
			return new Light(yrenderer::LightType::POINT, White);
		if (type == EdwardTag::_class)
			return new EdwardTag;
		if (type == ModelRef::_class)
			return new ModelRef;
		if (type == TerrainRef::_class)
			return new TerrainRef;
		if (type == Skeleton::_class)
			return new Skeleton;
		if (type == Animator::_class)
			return new Animator;
		if (type == SolidBody::_class)
			return new SolidBody;
		if (type == MeshCollider::_class)
			return new MeshCollider;
		if (type == TerrainCollider::_class)
			return new TerrainCollider;
		return reinterpret_cast<Component*>(type->create_instance());
		msg_error("new component..." + p2s(type));
		msg_write(type->name);
		return nullptr;
	};
	entity_manager->component_manager->f_apply = [] (const kaba::Class* type, Component* c, const base::map<string, Any>& var) {
		if (type == Light::_class) {
			auto l = static_cast<Light*>(c);
			if (var.contains("type"))
				l->light.type = (yrenderer::LightType)var["type"].to_i32();
			if (var.contains("radius") and var.contains("color"))
				l->light.light.col = any_to_color(var["color"]) * (var["radius"].to_f32() * var["radius"].to_f32() / 100);
			if (var.contains("power") and var.contains("color"))
				l->light.light.col = any_to_color(var["color"]) * var["power"].to_f32();
			if (var.contains("theta"))
				l->light.light.theta = var["theta"].to_f32();
		}
	};
	/*entity_manager->component_manager->f_parse_type = [] (const string& name) -> const kaba::Class* {
		const Array list = {Camera::_class, Light::_class, ModelRef::_class, TerrainRef::_class, Skeleton::_class, Animator::_class, SolidBody::_class, MeshCollider::_class, TerrainCollider::_class, EdwardTag::_class};
		for (const auto* t: list)
			if (t->name == name)
				return t;
		return nullptr;
	};*/
	reset();
}

DataWorld::~DataWorld() {
}


void DataWorld::MetaData::reset() {
	fog.enabled = false;
#if HAS_LIB_GL
	fog.mode = nix::FogMode::EXP;
#endif
	fog.start = 1;
	fog.end = 1000;
	fog.density = 0.0001f;
	fog.col = color(1, 0.8f, 0.8f, 0.8f).srgb_to_lin();

	skybox_files.clear();

	background_color = color(1, 0.2f, 0.4f, 0.6f).srgb_to_lin();

	physics_enabled = false;
	physics_mode = PhysicsMode::FULL_EXTERNAL;
	gravity = vec3(0, -981, 0);

	systems.clear();
	music_files.clear();
}


void DataWorld::reset() {
	filename = "";

	// delete old data...
	entity_manager->reset();
	links.clear();

	EgoIndex = -1;

	meta_data.reset();

	reset_history();
	out_changed();
}

void DataWorld::add_initial_data() {
	{
		auto e = add_entity({0,0,0}, quaternion::ID);
		auto cam = entity_manager->add_component<Camera>(e);
	}

	{
		auto e = add_entity({0,1000,0}, quaternion::rotation(vec3(pi/4,0,0)));
		auto sun = entity_manager->add_component<Light>(e);
		sun->light.enabled = true;
		sun->light.init(yrenderer::LightType::DIRECTIONAL, White);
		sun->light.light.harshness = 0.75;
	}

	reset_history();
	out_changed();
}


Box DataWorld::get_bounding_box() const {
	bool found_any = false;
	vec3 min, max;

	auto merge = [&] (const vec3& _min, const vec3& _max) {
		if (!found_any) {
			min = _min;
			max = _max;
		}
		min._min(_min);
		max._max(_max);
		found_any = true; //|=(_min!=_max);
	};

	for (auto mr: entity_manager->get_component_list_const<ModelRef>())
		if (auto m = mr->model) {
			vec3 min2 = mr->owner->pos - vec3(1,1,1) * m->prop.radius;
			vec3 max2 = mr->owner->pos + vec3(1,1,1) * m->prop.radius;
			merge(min2, max2);
		}
#if 0
	for (auto tr: entity_manager->get_component_list_const<TerrainRef>())
		if (auto t = tr->terrain) {
			//auto box = t->bounding_box();
			vec3 min2 = tr->owner->pos - vec3(1,1,1) * t->...;
			vec3 max2 = tr->owner->pos + vec3(1,1,1) * t->...;
			merge(min2, max2);
		}
#endif

	if (!found_any) {
		min = vec3(-100,-100,-100);
		max = vec3( 100, 100, 100);
	}
	return {min, max};
}

Entity *DataWorld::entity(int index) {
	return entity_manager->entities[index];
}


#define IMPLEMENT_COUNT_SELECTED(FUNC, ARRAY) \
	int DataWorld::FUNC() {                   \
		int n = 0;                            \
		for (auto &o: ARRAY)                  \
			if (o.is_selected)                \
				n ++;                         \
		return n;                             \
	}



void DataWorld::update_data() {
}

#if 0
WorldObject* DataWorld::add_object(const Path &filename, const vec3& pos) {
	WorldObject o;
	o.pos = pos;
	o.ang = v_0;//quaternion::ID;
	o.is_selected = true;
	o.filename = filename;
	o.object = session->resource_manager->load_model(filename);
	return (WorldObject*)execute(new ActionWorldAddObject(o));
}

WorldTerrain* DataWorld::add_terrain(const Path &filename, const vec3& pos) {
	return (WorldTerrain*)execute(new ActionWorldAddTerrain(pos, filename));
}

WorldTerrain* DataWorld::add_new_terrain(const vec3& pos, const vec3& size, int num_x, int num_z) {
	return (WorldTerrain*)execute(new ActionWorldAddTerrain(pos, size, num_x, num_z));
}

void DataWorld::edit_light(int index, const WorldLight& l) {
	execute(new ActionWorldEditLight(index, l));
}

/*WorldCamera* DataWorld::add_camera(const WorldCamera &c) {
	return (WorldCamera*)execute(new ActionWorldAddCamera(c));
}*/
#endif


void DataWorld::copy(LevelData& temp, const Data::Selection& sel) const {
	temp.entities.clear();

	if (sel.contains(MultiViewType::WORLD_ENTITY))
		for (auto&& [i, e]: enumerate(entity_manager->entities))
			if (sel[MultiViewType::WORLD_ENTITY].contains(i)) {
				LevelData::Entity ee;
				ee.pos = e->pos;
				ee.ang = e->ang;
				for (auto c: e->components)
					if (c->component_type != EdwardTag::_class)
						ee.components.add(session->plugin_manager->describe_class(c->component_type, c));
				for (const auto& c: e->get_component<EdwardTag>()->unknown_components)
					ee.components.add(c);

				temp.entities.add(ee);
			}
	if (sel.contains(MultiViewType::WORLD_LINK))
		for (const auto& [i, o]: enumerate(links))
			if (sel[MultiViewType::WORLD_LINK].contains(i)) {
				//links.add(o);
			}
}

bool DataWorld::is_empty() const {
	return entity_manager->entities.num == 0;
}

void DataWorld::paste(const LevelData& temp, Selection* selection) {
	execute(new ActionWorldPaste(temp));
	if (selection) {
		selection->clear();
		selection->set(MultiViewType::WORLD_ENTITY, {});
		for (int i=0; i < temp.entities.num; i++)
			(*selection)[MultiViewType::WORLD_ENTITY].add(entity_manager->entities.num - 1 - i);
	}
}

void DataWorld::delete_selection(const Selection& selection) {
	execute(new ActionWorldDeleteSelection(this, selection));
}

Entity* DataWorld::add_entity(const vec3& pos, const quaternion& ang) {
	return static_cast<Entity*>(execute(new ActionWorldAddEntity(pos, ang)));
}

void DataWorld::edit_entity(Entity* e, const vec3& pos, const quaternion& ang) {
	execute(new ActionWorldEditBaseEntity(entity_manager->entity_index(e), pos, ang));
}

/*void DataWorld::edit_camera(int index, const WorldCamera& c) {
	execute(new ActionWorldEditCamera(index, c));
}*/

void DataWorld::edit_terrain_meta_data(int index, const vec3& pattern) {
	execute(new ActionWorldEditTerrainMetaData(index, pattern));
}


Component* DataWorld::entity_add_component_generic(Entity* e, const kaba::Class* type, const base::map<string, Any>& variables) {
	return static_cast<Component*>(execute(new ActionWorldAddComponent(entity_manager->entity_index(e), type, variables)));
}
void DataWorld::entity_remove_component(Entity* e, const kaba::Class* type) {
	execute(new ActionWorldRemoveComponent(entity_manager->entity_index(e), type));
}
void DataWorld::entity_edit_component(Entity* e, const kaba::Class* type, const ScriptInstanceData& c) {
	execute(new ActionWorldEditComponent(entity_manager->entity_index(e), type, c));
}

void DataWorld::entity_remove_unknown_component(Entity* e, int cindex) {
	execute(new ActionWorldRemoveUnknownComponent(entity_manager->entity_index(e), cindex));
}
void DataWorld::entity_edit_unknown_component(Entity* e, int cindex, const ScriptInstanceData& c) {
	//execute(new ActionWorldEditUnknownComponent(index, cindex, c));
}

Entity *DataWorld::_create_entity(const vec3 &pos, const quaternion &ang) {
	auto e = entity_manager->create_entity(pos, ang);
	entity_manager->add_component<EdwardTag>(e);
	return e;
}


void DataWorld::_entity_apply_components(Entity *e, const Array<ScriptInstanceData> &components) {
	for (const auto& cc: components) {
		for (const auto c: session->plugin_manager->component_classes)
			if (cc.class_name == c->name) {
				auto comp = entity_manager->_add_component_generic_(e, c);
				session->plugin_manager->set_variables(comp, c, cc.variables);
				return;
			}

		msg_error("UNKNOWN COMPONENT: " + cc.class_name);
		auto tag = e->get_component<EdwardTag>();
		tag->unknown_components.add(cc);
	}
};


