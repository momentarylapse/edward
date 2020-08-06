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
#include "../../Mode/World/ModeWorld.h"
#include "../../Edward.h"
#include "../../Storage/Storage.h"
#include "../../x/object.h"
#include "../../x/terrain.h"
#include "../../x/world.h"
#include "../../meta.h"
#include "../../Action/World/Object/ActionWorldAddObject.h"
#include "../../Action/World/Terrain/ActionWorldAddTerrain.h"
#include "../../Action/World/ActionWorldPaste.h"
#include "../../Action/World/ActionWorldDeleteSelection.h"
#include "../../lib/nix/nix.h"
#include "../../MultiView/MultiView.h"
#include "../../x/ModelManager.h"



DataWorld::DataWorld() :
	Data(FD_WORLD)
{
	reset();
}

DataWorld::~DataWorld() {
}


void DataWorld::MetaData::reset() {
	fog.enabled = false;
	fog.mode = FOG_EXP;
	fog.start = 1;
	fog.end = 1000;
	fog.density = 0.0001f;
	fog.col = color(1, 0.8f, 0.8f, 0.8f);

	skybox_files.clear();
	skybox_files.resize(32);

	background_color = color(1, 0.2f, 0.4f, 0.6f);

	physics_enabled = false;
	gravity = vector(0, -981, 0);

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
	sun.pos = vector(0,1000,0);
	sun.ang = vector(-pi/4,0,0);
	sun.enabled = true;
	sun.type = LightType::DIRECTIONAL;
	sun.radius = 0;
	sun.theta = 0;
	sun.col = White;
	sun.harshness = 0.75;
	lights.add(sun);

	meta_data.reset();

	reset_history();
	notify();
}


void DataWorld::get_bounding_box(vector &min, vector &max) {
	bool found_any = false;

	for (WorldObject &o: objects)
		if (o.object) {
			vector min2 = o.pos - vector(1,1,1) * o.object->prop.radius;
			vector max2 = o.pos + vector(1,1,1) * o.object->prop.radius;
			if (!found_any) {
				min = min2;
				max = max2;
			}
			min._min(min2);
			max._max(max2);
			found_any = true; //|=(min2!=max2);
		}
	for (WorldTerrain &t: terrains)
		if (t.terrain) {
			vector min2 = t.terrain->min;
			vector max2 = t.terrain->max;
			if (!found_any) {
				min = min2;
				max = max2;
			}
			min._min(min2);
			max._max(max2);
			found_any = true;
		}
	if (!found_any) {
		min = vector(-100,-100,-100);
		max = vector( 100, 100, 100);
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

WorldObject* DataWorld::add_object(const Path &filename, const vector& pos) {
	WorldObject o;
	o.pos = pos;
	o.ang = v_0;//quaternion::ID;
	o.is_selected = true;
	o.filename = filename;
	o.view_stage = 0;//mode_world->ViewStage;
	o.object = (Object*)ModelManager::load(filename);
	return (WorldObject*)execute(new ActionWorldAddObject(o));
}

WorldTerrain* DataWorld::add_terrain(const Path &filename, const vector& pos)
{	return (WorldTerrain*)execute(new ActionWorldAddTerrain(pos, filename));	}

WorldTerrain* DataWorld::add_new_terrain(const vector& pos, const vector& size, int num_x, int num_z)
{	return (WorldTerrain*)execute(new ActionWorldAddTerrain(pos, size, num_x, num_z));	}


void DataWorld::clear_selection() {
	for (WorldObject &o: objects)
		o.is_selected = false;
	for (WorldTerrain &t: terrains)
		t.is_selected = false;
}


void DataWorld::copy(Array<WorldObject> &_objects, Array<WorldTerrain> &_terrains) {
	_objects.clear();
	_terrains.clear();

	for (WorldObject &o: objects)
		if (o.is_selected)
			_objects.add(o);
	for (WorldTerrain &t: terrains)
		if (t.is_selected)
			_terrains.add(t);
}

void DataWorld::paste(Array<WorldObject> &o, Array<WorldTerrain> &t)
{	execute(new ActionWorldPaste(o, t));	}

void DataWorld::delete_selection()
{	execute(new ActionWorldDeleteSelection());	}
