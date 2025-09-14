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

string ScriptInstanceData::get(const string &name) const {
	for (const auto& v: variables)
		if (v.name == name)
			return v.value;
	return "";
}

void ScriptInstanceData::set(const string &name, const string &type, const string &value) {
	for (auto& v: variables)
		if (v.name == name) {
			v.value = value;
			v.type = type;
			return;
		}
	variables.add({name, type, value});
}

ScriptInstanceData& WorldEntity::get(const string& class_name) {
	for (auto& c: components)
		if (c.class_name == class_name)
			return c;
	static ScriptInstanceData dummy;
	dummy.class_name = "";
	return dummy;
}

ScriptInstanceData& EdwardTag::get(const string& class_name) {
	for (auto& c: user_components)
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
	entity_manager->component_manager->factory = [] (const kaba::Class* type, const base::map<string, Any>& var) -> Component* {
		if (type == Camera::_class)
			return new Camera();
		if (type == Light::_class) {
			yrenderer::LightType type = var.contains("type") ? (yrenderer::LightType)var["type"].to_i32() : yrenderer::LightType::POINT;
			float radius = var.contains("radius") ? var["radius"].to_f32() : 100.0f;
			float theta = var.contains("theta") ? var["theta"].to_f32() : 0.0f;
			return new Light(type, White * (radius * radius / 100), theta);
		}
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
		msg_error("new component..." + p2s(type));
		msg_write(type->name);
		return nullptr;
	};
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
	for (int i=0;i<entities.num;i++)
		if (entities[i].object.object)
			delete entities[i].object.object;

	entities.clear();
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
	for (auto&& [i, e]: enumerate(entities)) {
		if (e.basic_type == MultiViewType::WORLD_OBJECT)
			e.object.update_data();
		if (e.basic_type == MultiViewType::WORLD_TERRAIN)
			e.terrain.update_data();
	}
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


void DataWorld::copy(DataWorld& temp, const Data::Selection& sel) const {
	temp.entities.clear();

	for (auto&& [i, o]: enumerate(entities))
		if (sel[MultiViewType::WORLD_ENTITY].contains(i))
			temp.entities.add(o);
}

bool DataWorld::is_empty() const {
	return entities.num == 0;
}

void DataWorld::paste(const DataWorld& temp) {
	execute(new ActionWorldPaste(temp));
}

void DataWorld::delete_selection(const Selection& selection) {
	execute(new ActionWorldDeleteSelection(this, selection));
}

Entity* DataWorld::add_entity(const vec3& pos, const quaternion& ang) {
	return static_cast<Entity*>(execute(new ActionWorldAddEntity(pos, ang)));
}

void DataWorld::edit_entity(int index, const vec3& pos, const quaternion& ang) {
	execute(new ActionWorldEditBaseEntity(index, pos, ang));
}

/*void DataWorld::edit_camera(int index, const WorldCamera& c) {
	execute(new ActionWorldEditCamera(index, c));
}*/

void DataWorld::edit_terrain_meta_data(int index, const vec3& pattern) {
	execute(new ActionWorldEditTerrainMetaData(index, pattern));
}


Component* DataWorld::entity_add_component_generic(int index, const kaba::Class* _class, const base::map<string, Any>& variables) {
	return static_cast<Component*>(execute(new ActionWorldAddComponent(index, _class, variables)));
}
void DataWorld::entity_remove_component(int index, const kaba::Class* type) {
	execute(new ActionWorldRemoveComponent(index, type));
}
void DataWorld::entity_edit_component(int index, const kaba::Class* type, const ScriptInstanceData& c) {
	execute(new ActionWorldEditComponent(index, type, c));
}

void DataWorld::entity_add_user_component(int index, const ScriptInstanceData& c) {
	execute(new ActionWorldAddUserComponent(index, c));
}
void DataWorld::entity_remove_user_component(int index, int cindex) {
	execute(new ActionWorldRemoveUserComponent(index, cindex));
}
void DataWorld::entity_edit_user_component(int index, int cindex, const ScriptInstanceData& c) {
	//execute(new ActionWorldEditUserComponent(index, cindex, c));
}

#if 0
Data::Selection DataWorld::get_selection() const {
	Selection s;
	s.add({MultiViewType::WORLD_ENTITY, {}});
	s.add({MultiViewType::WORLD_OBJECT, {}});
	s.add({MultiViewType::WORLD_TERRAIN, {}});
	s.add({MultiViewType::WORLD_CAMERA, {}});
	s.add({MultiViewType::WORLD_LINK, {}});
	s.add({MultiViewType::WORLD_LIGHT, {}});
	for (const auto& [i, o]: enumerate(entities))
		if (o.is_selected)
			s[MultiViewType::WORLD_ENTITY].add(i);
	for (const auto& [i, o]: enumerate(links))
		if (o.is_selected)
			s[MultiViewType::WORLD_LINK].add(i);
	return s;
}
#endif

