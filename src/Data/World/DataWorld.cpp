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
#include "../../x/model_manager.h"
#include "../../x/terrain.h"
#include "../../x/world.h"
#include "../../meta.h"
#include "../../Action/World/Object/ActionWorldAddObject.h"
#include "../../Action/World/Terrain/ActionWorldAddTerrain.h"
#include "../../Action/World/ActionWorldPaste.h"
#include "../../Action/World/ActionWorldDeleteSelection.h"
#include "../../lib/nix/nix.h"
#include "../../MultiView/MultiView.h"


void WorldObject::UpdateData()
{
	if (!object)
		return;
	object->pos = pos;
	object->ang = quaternion::rotation_v(Ang);
	object->UpdateMatrix();
}

bool WorldTerrain::Load(const vector &_pos, const string &filename, bool deep)
{
	view_stage = 0;
	is_selected = false;
	is_special = false;

	FileName = filename.substr(MapDir.num, -1);
	FileName.resize(FileName.num - 4);

	terrain = new Terrain();
	bool Error = !terrain->Load(FileName, _pos, deep);

	if (Error){
		delete(terrain);
		terrain = NULL;
	}

	return !Error;
}

bool WorldTerrain::Save(const string &filename)
{
	FileName = filename.substr(MapDir.num, -1);
	FileName.resize(FileName.num - 4);


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

void WorldTerrain::UpdateData()
{
	if (!terrain)
		return;
	terrain->pos = pos;
}

DataWorld::DataWorld() :
	Data(FD_WORLD)
{
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

	Ambient = color(1, 0.25f, 0.25f, 0.25f);

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
	for (int i=0;i<Objects.num;i++)
		if (Objects[i].object)
			delete(Objects[i].object);

	Objects.clear();
	Terrains.clear();

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
	sun.mode = LightMode::DIRECTIONAL;
	sun.radius = 0;
	sun.ambient = color(1, 0.05f, 0.05f, 0.05f);
	sun.diffuse= color(1, 0.8f, 0.8f, 0.8f);
	sun.specular = Black;
	lights.add(sun);

	meta_data.Reset();

	reset_history();
	notify();
}


void DataWorld::GetBoundaryBox(vector &min, vector &max)
{
	bool found_any=false;

	for (WorldObject &o: Objects)
		if (o.object){
			vector min2 = o.pos - vector(1,1,1) * o.object->radius;
			vector max2 = o.pos + vector(1,1,1) * o.object->radius;
			if (!found_any){
				min = min2;
				max = max2;
			}
			min._min(min2);
			max._max(max2);
			found_any = true; //|=(min2!=max2);
		}
	for (WorldTerrain &t: Terrains)
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

IMPLEMENT_COUNT_SELECTED(GetSelectedObjects, Objects)
IMPLEMENT_COUNT_SELECTED(GetSelectedTerrains, Terrains)
IMPLEMENT_COUNT_SELECTED(get_selected_lights, lights)
IMPLEMENT_COUNT_SELECTED(get_selected_cameras, cameras)


void DataWorld::UpdateData() {
	foreachi(auto &o, Objects, i){
		o.UpdateData();
		o.is_special = (i == EgoIndex);
	}
	for (auto &t: Terrains)
		t.UpdateData();
}

WorldObject* DataWorld::AddObject(const string& filename, const vector& pos)
{	return (WorldObject*)execute(new ActionWorldAddObject(filename, pos));	}

WorldTerrain* DataWorld::AddTerrain(const string& filename, const vector& pos)
{	return (WorldTerrain*)execute(new ActionWorldAddTerrain(pos, filename));	}

WorldTerrain* DataWorld::AddNewTerrain(const vector& pos, const vector& size, int num_x, int num_z)
{	return (WorldTerrain*)execute(new ActionWorldAddTerrain(pos, size, num_x, num_z));	}


void DataWorld::ClearSelection()
{
	for (WorldObject &o: Objects)
		o.is_selected = false;
	for (WorldTerrain &t: Terrains)
		t.is_selected = false;
}


void DataWorld::Copy(Array<WorldObject> &objects, Array<WorldTerrain> &terrains)
{
	objects.clear();
	terrains.clear();

	for (WorldObject &o: Objects)
		if (o.is_selected)
			objects.add(o);
	for (WorldTerrain &t: Terrains)
		if (t.is_selected)
			terrains.add(t);
}

void DataWorld::Paste(Array<WorldObject> &objects, Array<WorldTerrain> &terrains)
{	execute(new ActionWorldPaste(objects, terrains));	}

void DataWorld::DeleteSelection()
{	execute(new ActionWorldDeleteSelection());	}
