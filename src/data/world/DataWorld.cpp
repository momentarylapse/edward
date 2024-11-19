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
#include "../../action/world/camera/ActionWorldEditCamera.h"
#include "../../action/world/light/ActionWorldAddLight.h"
#include "../../action/world/light/ActionWorldEditLight.h"
#include "../../action/world/light/ActionWorldDeleteLight.h"
#include "../../action/world/object/ActionWorldAddObject.h"
#include "../../action/world/terrain/ActionWorldAddTerrain.h"
#include "../../action/world/ActionWorldPaste.h"
#include "../../action/world/ActionWorldDeleteSelection.h"
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

	scripts.clear();
	music_files.clear();
}


void DataWorld::reset() {
	filename = "";

	// delete old data...
	for (int i=0;i<objects.num;i++)
		if (objects[i].object)
			delete(objects[i].object);

	objects.clear();
	terrains.clear();
	links.clear();

	EgoIndex = -1;

	cameras.clear();
	WorldCamera cam;
	cam.ang = v_0;
	cameras.add(cam);

	lights.clear();
	WorldLight sun;
	sun.pos = vec3(0,1000,0);
	sun.ang = vec3(pi/4,0,0);
	sun.enabled = true;
	sun.type = LightType::DIRECTIONAL;
	sun.radius = 0;
	sun.theta = 0;
	sun.col = White;
	sun.harshness = 0.75;
	lights.add(sun);

	meta_data.reset();

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

	for (WorldObject &o: objects)
		if (o.object) {
			vec3 min2 = o.pos - vec3(1,1,1) * o.object->prop.radius;
			vec3 max2 = o.pos + vec3(1,1,1) * o.object->prop.radius;
			merge(min2, max2);
		}
	for (WorldTerrain &t: terrains)
		if (t.terrain) {
			auto box = t.bounding_box();
			merge(box.min, box.max);
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

IMPLEMENT_COUNT_SELECTED(get_selected_objects, objects)
IMPLEMENT_COUNT_SELECTED(get_selected_terrains, terrains)
IMPLEMENT_COUNT_SELECTED(get_selected_lights, lights)
IMPLEMENT_COUNT_SELECTED(get_selected_cameras, cameras)


void DataWorld::update_data() {
	foreachi(auto &o, objects, i){
		o.update_data();
		o.is_special = (i == EgoIndex);
	}
	for (auto &t: terrains)
		t.update_data();
}

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

WorldLight* DataWorld::add_light(const WorldLight &l) {
	return (WorldLight*)execute(new ActionWorldAddLight(l));
}

void DataWorld::edit_light(int index, const WorldLight& l) {
	execute(new ActionWorldEditLight(index, l));
}

/*WorldCamera* DataWorld::add_camera(const WorldCamera &c) {
	return (WorldCamera*)execute(new ActionWorldAddCamera(c));
}*/

void DataWorld::edit_camera(int index, const WorldCamera& c) {
	execute(new ActionWorldEditCamera(index, c));
}


void DataWorld::clear_selection() {
	for (WorldObject &o: objects)
		o.is_selected = false;
	for (WorldTerrain &t: terrains)
		t.is_selected = false;
}


void DataWorld::copy(Array<WorldObject> &_objects, Array<WorldTerrain> &_terrains, Array<WorldCamera> &_cameras, Array<WorldLight> &_lights) {
	_objects.clear();
	_terrains.clear();
	_cameras.clear();
	_lights.clear();

	for (auto &o: objects)
		if (o.is_selected)
			_objects.add(o);
	for (auto &t: terrains)
		if (t.is_selected)
			_terrains.add(t);
	for (auto &c: cameras)
		if (c.is_selected)
			_cameras.add(c);
	for (auto &l: lights)
		if (l.is_selected)
			_lights.add(l);
}

void DataWorld::paste(Array<WorldObject> &o, Array<WorldTerrain> &t, Array<WorldCamera> &c, Array<WorldLight> &l)
{	execute(new ActionWorldPaste(o, t, c, l));	}

void DataWorld::delete_selection()
{	execute(new ActionWorldDeleteSelection());	}
