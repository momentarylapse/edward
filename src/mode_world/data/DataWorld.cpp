/*
 * DataWorld.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "DataWorld.h"
#include "WorldLight.h"
#include "WorldObject.h"
#include "WorldTerrain.h"
#include "WorldCamera.h"
#include "WorldLink.h"
#if HAS_LIB_GL
#include "../../mode/world/ModeWorld.h"
#endif
#include "../../Session.h"
#include "../../storage/Storage.h"
#include <y/world/Model.h>
#include <y/world/Terrain.h>
#include <y/world/World.h>
#include <y/meta.h>
//#include "../action/camera/ActionWorldEditCamera.h"
#include "../action/light/ActionWorldAddLight.h"
#include "../action/entity/ActionWorldEditComponent.h"
//#include "../action/light/ActionWorldEditLight.h"
//#include "../action/light/ActionWorldDeleteLight.h"
#include "../action/object/ActionWorldAddObject.h"
#include "../action/terrain/ActionWorldAddTerrain.h"
#include "../action/ActionWorldPaste.h"
#include <lib/os/msg.h>
#include <lib/base/iter.h>
#include <mode_world/action/entity/ActionWorldAddEntity.h>

#include "../action/ActionWorldDeleteSelection.h"
#include <y/graphics-impl.h>
#include <y/helper/ResourceManager.h>



DataWorld::DataWorld(Session *s) :
	Data(s, FD_WORLD)
{
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
	WorldEntity cam;
	cam.basic_type = MultiViewType::WORLD_CAMERA;
	entities.add(cam);

	WorldEntity sun;
	sun.basic_type = MultiViewType::WORLD_LIGHT;
	sun.pos = vec3(0,1000,0);
	sun.ang = quaternion::rotation(vec3(pi/4,0,0));
	sun.light.enabled = true;
	sun.light.type = LightType::DIRECTIONAL;
	sun.light.radius = 0;
	sun.light.theta = 0;
	sun.light.col = White;
	sun.light.harshness = 0.75;
	entities.add(sun);

	reset_history();
	out_changed();
}


void DataWorld::get_bounding_box(vec3 &min, vec3 &max) {
	bool found_any = false;

	auto merge = [&] (const vec3& _min, const vec3& _max) {
		if (!found_any) {
			min = _min;
			max = _max;
		}
		min._min(_min);
		max._max(_max);
		found_any = true; //|=(_min!=_max);
	};

	for (const auto &e: entities)
		if (e.basic_type == MultiViewType::WORLD_OBJECT) {
			if (e.object.object) {
				vec3 min2 = e.pos - vec3(1,1,1) * e.object.object->prop.radius;
				vec3 max2 = e.pos + vec3(1,1,1) * e.object.object->prop.radius;
				merge(min2, max2);
			}
		} else if (e.basic_type == MultiViewType::WORLD_TERRAIN) {
			if (e.terrain.terrain) {
				auto box = e.terrain.bounding_box();
				merge(box.min, box.max);
			}
		}
	if (!found_any) {
		min = vec3(-100,-100,-100);
		max = vec3( 100, 100, 100);
	}
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
		e.is_special = (i == EgoIndex);

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


void DataWorld::clear_selection() {
	for (auto& o: entities)
		o.is_selected = false;
}


void DataWorld::copy(DataWorld& temp) const {
	temp.entities.clear();

	for (auto &o: entities)
		if (o.is_selected)
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

WorldEntity* DataWorld::add_entity(const WorldEntity& e) {
	return (WorldEntity*)execute(new ActionWorldAddEntity(e));
}

void DataWorld::edit_light(int index, const WorldLight& l) {
	execute(new ActionWorldEditLight(index, l));
}

void DataWorld::edit_camera(int index, const WorldCamera& c) {
	execute(new ActionWorldEditCamera(index, c));
}


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

