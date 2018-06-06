/*
 * DataWorld.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "DataWorld.h"
#include "../../Mode/World/ModeWorld.h"
#include "../../Edward.h"
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
	object->ang = Ang;
	object->UpdateMatrix();
}

bool WorldTerrain::Load(const vector &_pos, const string &filename, bool deep)
{
	view_stage = 0;
	is_selected = false;
	is_special = false;

	if (deep)
		ed->makeDirs(filename);
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
	ed->makeDirs(filename);
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

bool DataWorld::save(const string & _filename)
{
/*	if (!SaveTerrains())
		return;*/
	filename = _filename;
	ed->makeDirs(filename);
	File *f = FileCreateText(filename);
	f->float_decimals = 6;

	f->WriteFileFormatVersion(false, 10);
	f->write_comment("// Terrains");
	f->write_int(Terrains.num);
	for (WorldTerrain &t: Terrains){
		f->write_str(t.FileName);
		f->write_float(t.pos.x);
		f->write_float(t.pos.y);
		f->write_float(t.pos.z);
	}
	f->write_comment("// Gravitation");
	f->write_float(meta_data.Gravity.x);
	f->write_float(meta_data.Gravity.y);
	f->write_float(meta_data.Gravity.z);
	f->write_comment("// EgoIndex");
	f->write_int(EgoIndex);
	f->write_comment("// Background");
	write_color_argb(f, meta_data.BackGroundColor);
	int ns = 0;
	foreachi(string &sb, meta_data.SkyBoxFile, i)
		if (sb.num > 0)
			ns = i + 1;
	f->write_int(ns);
	for (int i=0;i<ns;i++)
		f->write_str(meta_data.SkyBoxFile[i]);
	f->write_comment("// Fog");
	f->write_bool(meta_data.FogEnabled);
	f->write_word(meta_data.FogMode);
	f->write_float(meta_data.FogStart);
	f->write_float(meta_data.FogEnd);
	f->write_float(meta_data.FogDensity);
	write_color_argb(f, meta_data.FogColor);
	f->write_comment("// Music");
	f->write_int(meta_data.MusicFile.num);
	for (string &m: meta_data.MusicFile)
		f->write_str(m);
	f->write_comment("// Objects");
	f->write_int(Objects.num);
	for (WorldObject &o: Objects){
		f->write_str(o.FileName);
		f->write_str(o.Name);
		f->write_float(o.pos.x);
		f->write_float(o.pos.y);
		f->write_float(o.pos.z);
		f->write_float(o.Ang.x);
		f->write_float(o.Ang.y);
		f->write_float(o.Ang.z);
	}
	f->write_comment("// Scripts");
	f->write_int(meta_data.scripts.num);
	for (auto &s: meta_data.scripts){
		f->write_str(s.filename);
		f->write_int(s.variables.num);
		for (auto &v: s.variables){
			f->write_str(v.name);
			f->write_str(v.value);
		}
	}
	f->write_comment("// ScriptVars");
	f->write_int(0);
	f->write_comment("// Sun");
	f->write_bool(meta_data.SunEnabled);
	write_color_3i(f, meta_data.SunAmbient);
	write_color_3i(f, meta_data.SunDiffuse);
	write_color_3i(f, meta_data.SunSpecular);
	f->write_float(meta_data.SunAng.x);
	f->write_float(meta_data.SunAng.y);
	f->write_comment("// Ambient");
	write_color_3i(f, meta_data.Ambient);
	f->write_comment("// Physics");
	f->write_bool(meta_data.PhysicsEnabled);
	f->write_int(0);
	f->write_comment("#");

	delete(f);

	action_manager->markCurrentAsSave();
	return true;
}



bool DataWorld::load(const string & _filename, bool deep)
{
	int ffv;

	reset();

	filename = _filename;
	if (this == mode_world->data)
		ed->makeDirs(filename);

	File *f = NULL;

	try{

		f = FileOpenText(filename);
	file_time = f->GetDateModification().time;

	ffv = f->ReadFileFormatVersion();

	if ((ffv==10) or (ffv==9)){ // new format
		// Terrains
		f->read_comment();
		int n = f->read_int();
		for (int i=0;i<n;i++){
			WorldTerrain t;
			t.FileName = f->read_str();
			f->read_vector(&t.pos);
			Terrains.add(t);
		}
		// Gravitation
		f->read_comment();
		meta_data.Gravity.x = f->read_float();
		meta_data.Gravity.y = f->read_float();
		meta_data.Gravity.z = f->read_float();
		// EgoIndex
		f->read_comment();
		EgoIndex = f->read_int();
		// Background
		f->read_comment();
		if (ffv == 9)
			f->read_bool(); // BackGroundColorEnabled
		read_color_argb(f,meta_data.BackGroundColor);
		if (ffv==9){
			meta_data.SkyBoxFile[0] = f->read_str();
		}else{
			int ns=f->read_int();
			if (ns > meta_data.SkyBoxFile.num)
				meta_data.SkyBoxFile.resize(ns);
			for (int i=0;i<ns;i++)
				meta_data.SkyBoxFile[i] = f->read_str();
		}
		// Fog
		f->read_comment();
		meta_data.FogEnabled = f->read_bool();
		meta_data.FogMode = f->read_word();
		meta_data.FogStart = f->read_float();
		meta_data.FogEnd = f->read_float();
		meta_data.FogDensity = f->read_float();
		read_color_argb(f, meta_data.FogColor);
		// Music
		f->read_comment();
		n = f->read_int();
		for (int i=0;i<n;i++)
			meta_data.MusicFile.add(f->read_str());
		// Objects
		f->read_comment();
		n = f->read_int();
		for (int i=0;i<n;i++){
			WorldObject o;
			o.FileName = f->read_str();
			o.Name = f->read_str();
			o.pos.x = f->read_float();
			o.pos.y = f->read_float();
			o.pos.z = f->read_float();
			o.Ang.x = f->read_float();
			o.Ang.y = f->read_float();
			o.Ang.z = f->read_float();
			o.object = NULL;
			o.view_stage = 0;
			o.is_selected = false;
			o.is_special = false;
			Objects.add(o);
		}
		// Scripts
		f->read_comment();
		n = f->read_int();
		for (int i=0;i<n;i++){
			WorldScript s;
			s.filename = f->read_str();
			int nv = f->read_int();
			for (int j=0; j<nv; j++){
				WorldScriptVariable var;
				var.name = f->read_str();
				var.value = f->read_str();
				s.variables.add(var);
			}
			meta_data.scripts.add(s);
		}
		// ScriptVars
		f->read_comment();
		n = f->read_int();
		for (int i=0;i<n;i++)
			f->read_float();
		if (f->read_str() != "#"){
			meta_data.SunEnabled = f->read_bool();
			read_color_3i(f, meta_data.SunAmbient);
			read_color_3i(f, meta_data.SunDiffuse);
			read_color_3i(f, meta_data.SunSpecular);
			meta_data.SunAng.x = f->read_float();
			meta_data.SunAng.y = f->read_float();
			f->read_comment();
			read_color_3i(f, meta_data.Ambient);
			if (f->read_str() != "#"){
				meta_data.PhysicsEnabled = f->read_bool();
			}
		}

	}else{
		throw Exception(format(_("Falsches Datei-Format der Datei '%s': %d (statt %d - %d)"), filename.c_str(), ffv, 8, 10));
	}
	FileClose(f);

	if (deep){
		for (int i=0;i<Terrains.num;i++){
			ed->progress->set(_("Terrains"), (float)i / (float)Terrains.num / 2.0f);
			Terrains[i].Load(Terrains[i].pos, MapDir + Terrains[i].FileName + ".map", true);
		}
		for (int i=0;i<Objects.num;i++){
			ed->progress->set(format(_("Objekt %d von %d"), i, Objects.num), (float)i / (float)Objects.num / 2.0f + 0.5f);
			Objects[i].object = (Object*)LoadModel(Objects[i].FileName);
			Objects[i].object->pos = Objects[i].pos;
			Objects[i].object->ang = Objects[i].Ang;
//			if (Objects[i].object)
//				GodRegisterModel(Objects[i].object);
		}
	}




	// debug...
	/*if (JustCreateLightMap){
		for (int i=0;i<Object.num;i++)
			Object[i].is_selected = true;
		CreateLightMap(true);
	}*/
	resetHistory();
	notify();

	}catch(Exception &e){
		FileClose(f);
		msg_error(e.message());

		//ed->errorBox
		return false;
	}

	return true;
}


void DataWorld::MetaData::Reset()
{
	FogEnabled = false;
	FogMode = FOG_EXP;
	FogStart = 1;
	FogEnd = 1000;
	FogDensity = 0.0001f;
	FogColor = color(1, 0.8f, 0.8f, 0.8f);

	SunAng=vector(-pi/4,0,0);
	SunEnabled=true;
	SunAmbient = color(1, 0.05f, 0.05f, 0.05f);
	SunDiffuse= color(1, 0.8f, 0.8f, 0.8f);
	SunSpecular = Black;
	Ambient = color(1, 0.25f, 0.25f, 0.25f);

	SkyBoxFile.clear();
	SkyBoxFile.resize(32);

	BackGroundColor = color(1, 0.2f, 0.4f, 0.6f);

	PhysicsEnabled = false;
	Gravity = vector(0, -981, 0);

	scripts.clear();
	MusicFile.clear();
}


void DataWorld::MetaData::ApplyToDraw()
{
	nix::SetFog(FogMode, FogStart, FogEnd, FogDensity, FogColor);
	nix::EnableFog(FogEnabled);
	nix::SetLightDirectional(ed->multi_view_3d->light, -SunAng.ang2dir(), SunDiffuse, 0.5f, 0.8f);
	nix::EnableLight(ed->multi_view_3d->light, SunEnabled);
	nix::SetAmbientLight(Ambient);
}

void DataWorld::MetaData::DrawBackground()
{
	nix::ResetToColor(BackGroundColor);
	/*NixSetZ(false,false);
	NixSetWire(false);
	NixSetColor(BackGroundColor);
	NixDraw2D(r_id, NixTargetRect, 0);
	NixSetWire(ed->multi_view_3d->wire_mode);
	NixSetZ(true,true);*/
}


void DataWorld::reset()
{
	filename = "";

	// delete old data...
	for (int i=0;i<Objects.num;i++)
		if (Objects[i].object)
			delete(Objects[i].object);

	Objects.clear();
	Terrains.clear();

	EgoIndex = -1;

	meta_data.Reset();

	resetHistory();
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

int DataWorld::GetSelectedObjects()
{
	int n = 0;
	for (WorldObject &o: Objects)
		if (o.is_selected)
			n ++;
	return n;
}



int DataWorld::GetSelectedTerrains()
{
	int n = 0;
	for (WorldTerrain &t: Terrains)
		if (t.is_selected)
			n ++;
	return n;
}


void DataWorld::UpdateData()
{
	foreachi(WorldObject &o, Objects, i){
		o.UpdateData();
		o.is_special = (i == EgoIndex);
	}
	for (WorldTerrain &t: Terrains)
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
