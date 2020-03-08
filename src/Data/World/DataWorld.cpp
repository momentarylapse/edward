/*
 * DataWorld.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "DataWorld.h"
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


void WorldObject::update_data() {
	if (!object)
		return;
	object->pos = pos;
	object->ang = quaternion::rotation_v(ang);
	object->update_matrix();
}

bool WorldTerrain::load(const vector &_pos, const string &_filename, bool deep) {
	view_stage = 0;
	is_selected = false;
	is_special = false;

	filename = _filename.substr(engine.map_dir.num, -1);
	filename.resize(filename.num - 4);

	terrain = new Terrain();
	bool Error = !terrain->load(filename, _pos, deep);

	if (Error) {
		delete(terrain);
		terrain = NULL;
	}

	return !Error;
}

bool WorldTerrain::save(const string &_filename) {
	filename = _filename.substr(engine.map_dir.num, -1);
	filename.resize(filename.num - 4);


	File *f = NULL;

	try{
		f = FileCreate(filename);

	f->WriteFileFormatVersion(true, 4);
	f->write_byte(0);

	// Metrics
	f->write_comment("// Metrics");
	f->write_int(terrain->num_x);
	f->write_int(terrain->num_z);
	f->write_float(terrain->pattern.x);
	f->write_float(terrain->pattern.z);

	// Textures
	f->write_comment("// Textures");
	f->write_int(terrain->material->textures.num);
	for (int i=0;i<terrain->material->textures.num;i++){
		f->write_str(terrain->texture_file[i]);
		f->write_float(terrain->texture_scale[i].x);
		f->write_float(terrain->texture_scale[i].z);
	}
	f->write_str(terrain->material_file);

	// height
	for (int x=0;x<terrain->num_x+1;x++)
		for (int z=0;z<terrain->num_z+1;z++)
			f->write_float(terrain->height[x*(terrain->num_z+1) + z]);

	FileClose(f);

	}catch(Exception &e){}

	return true;
}

void WorldTerrain::update_data() {
	if (!terrain)
		return;
	terrain->pos = pos;
}

WorldCamera::WorldCamera() {
	fov = pi/4;
	min_depth = 1;
	max_depth = 10000;
	exposure = 1;
}

color WorldLight::ambient() {
	return col * ((1 - harshness) / 2);
}

color WorldLight::diffuse() {
	return col * harshness;
}

DataWorld::DataWorld() :
	Data(FD_WORLD)
{
	reset();
}

DataWorld::~DataWorld()
{
}


void DataWorld::MetaData::Reset()
{
	FogEnabled = false;
	FogMode = FOG_EXP;
	FogStart = 1;
	FogEnd = 1000;
	FogDensity = 0.0001f;
	FogColor = color(1, 0.8f, 0.8f, 0.8f);

	SkyBoxFile.clear();
	SkyBoxFile.resize(32);

	BackGroundColor = color(1, 0.2f, 0.4f, 0.6f);

	PhysicsEnabled = false;
	Gravity = vector(0, -981, 0);

	scripts.clear();
	MusicFile.clear();
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

	meta_data.Reset();

	reset_history();
	notify();
}


void DataWorld::GetBoundaryBox(vector &min, vector &max)
{
	bool found_any=false;

	for (WorldObject &o: objects)
		if (o.object){
			vector min2 = o.pos - vector(1,1,1) * o.object->prop.radius;
			vector max2 = o.pos + vector(1,1,1) * o.object->prop.radius;
			if (!found_any){
				min = min2;
				max = max2;
			}
			min._min(min2);
			max._max(max2);
			found_any = true; //|=(min2!=max2);
		}
	for (WorldTerrain &t: terrains)
		if (t.terrain){
			vector min2 = t.terrain->min;
			vector max2 = t.terrain->max;
			if (!found_any){
				min = min2;
				max = max2;
			}
			min._min(min2);
			max._max(max2);
			found_any = true;
		}
	if (!found_any){
		min=vector(-100,-100,-100);
		max=vector( 100, 100, 100);
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

IMPLEMENT_COUNT_SELECTED(GetSelectedObjects, objects)
IMPLEMENT_COUNT_SELECTED(GetSelectedTerrains, terrains)
IMPLEMENT_COUNT_SELECTED(get_selected_lights, lights)
IMPLEMENT_COUNT_SELECTED(get_selected_cameras, cameras)


void DataWorld::UpdateData() {
	foreachi(auto &o, objects, i){
		o.update_data();
		o.is_special = (i == EgoIndex);
	}
	for (auto &t: terrains)
		t.update_data();
}

WorldObject* DataWorld::AddObject(const string& filename, const vector& pos)
{	return (WorldObject*)execute(new ActionWorldAddObject(filename, pos));	}

WorldTerrain* DataWorld::AddTerrain(const string& filename, const vector& pos)
{	return (WorldTerrain*)execute(new ActionWorldAddTerrain(pos, filename));	}

WorldTerrain* DataWorld::AddNewTerrain(const vector& pos, const vector& size, int num_x, int num_z)
{	return (WorldTerrain*)execute(new ActionWorldAddTerrain(pos, size, num_x, num_z));	}


void DataWorld::ClearSelection()
{
	for (WorldObject &o: objects)
		o.is_selected = false;
	for (WorldTerrain &t: terrains)
		t.is_selected = false;
}


void DataWorld::Copy(Array<WorldObject> &_objects, Array<WorldTerrain> &_terrains) {
	_objects.clear();
	_terrains.clear();

	for (WorldObject &o: objects)
		if (o.is_selected)
			_objects.add(o);
	for (WorldTerrain &t: terrains)
		if (t.is_selected)
			_terrains.add(t);
}

void DataWorld::Paste(Array<WorldObject> &o, Array<WorldTerrain> &t)
{	execute(new ActionWorldPaste(o, t));	}

void DataWorld::DeleteSelection()
{	execute(new ActionWorldDeleteSelection());	}
