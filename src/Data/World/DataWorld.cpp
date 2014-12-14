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
	msg_db_f("Terrain.LoadFromFile", 1);
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
	msg_db_f("Terrain.LoadFromFile", 1);

	ed->makeDirs(filename);
	FileName = filename.substr(MapDir.num, -1);
	FileName.resize(FileName.num - 4);


	CFile *f = FileCreate(filename);
	if (!f)
		return false;

	f->WriteFileFormatVersion(true, 4);
	f->WriteByte(0);

	// Metrics
	f->WriteComment("// Metrics");
	f->WriteInt(terrain->num_x);
	f->WriteInt(terrain->num_z);
	f->WriteFloat(terrain->pattern.x);
	f->WriteFloat(terrain->pattern.z);

	// Textures
	f->WriteComment("// Textures");
	f->WriteInt(terrain->material->num_textures);
	for (int i=0;i<terrain->material->num_textures;i++){
		f->WriteStr(terrain->texture_file[i]);
		f->WriteFloat(terrain->texture_scale[i].x);
		f->WriteFloat(terrain->texture_scale[i].z);
	}
	f->WriteStr(terrain->material_file);

	// height
	for (int x=0;x<terrain->num_x+1;x++)
		for (int z=0;z<terrain->num_z+1;z++)
			f->WriteFloat(terrain->height[x*(terrain->num_z+1) + z]);

	FileClose(f);

	return true;
}

void WorldTerrain::UpdateData()
{
	if (!terrain)
		return;
	terrain->pos = pos;
}

DataWorld::DataWorld() :
	Data(FDWorld)
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
	CFile *f = FileCreate(filename);
	f->FloatDecimals = 6;

	f->WriteFileFormatVersion(false, 10);
	f->WriteComment("// Terrains");
	f->WriteInt(Terrains.num);
	foreach(WorldTerrain &t, Terrains){
		f->WriteStr(t.FileName);
		f->WriteFloat(t.pos.x);
		f->WriteFloat(t.pos.y);
		f->WriteFloat(t.pos.z);
	}
	f->WriteComment("// Gravitation");
	f->WriteFloat(meta_data.Gravity.x);
	f->WriteFloat(meta_data.Gravity.y);
	f->WriteFloat(meta_data.Gravity.z);
	f->WriteComment("// EgoIndex");
	f->WriteInt(EgoIndex);
	f->WriteComment("// Background");
	write_color_argb(f, meta_data.BackGroundColor);
	int ns = 0;
	foreachi(string &sb, meta_data.SkyBoxFile, i)
		if (sb.num > 0)
			ns = i + 1;
	f->WriteInt(ns);
	for (int i=0;i<ns;i++)
		f->WriteStr(meta_data.SkyBoxFile[i]);
	f->WriteComment("// Fog");
	f->WriteBool(meta_data.FogEnabled);
	f->WriteWord(meta_data.FogMode);
	f->WriteFloat(meta_data.FogStart);
	f->WriteFloat(meta_data.FogEnd);
	f->WriteFloat(meta_data.FogDensity);
	write_color_argb(f, meta_data.FogColor);
	f->WriteComment("// Music");
	f->WriteInt(meta_data.MusicFile.num);
	foreach(string &m, meta_data.MusicFile)
		f->WriteStr(m);
	f->WriteComment("// Objects");
	f->WriteInt(Objects.num);
	foreach(WorldObject &o, Objects){
		f->WriteStr(o.FileName);
		f->WriteStr(o.Name);
		f->WriteFloat(o.pos.x);
		f->WriteFloat(o.pos.y);
		f->WriteFloat(o.pos.z);
		f->WriteFloat(o.Ang.x);
		f->WriteFloat(o.Ang.y);
		f->WriteFloat(o.Ang.z);
	}
	f->WriteComment("// Scripts");
	f->WriteInt(meta_data.ScriptFile.num);
	foreach(string &s, meta_data.ScriptFile){
		f->WriteStr(s);
		f->WriteInt(0);
	}
	f->WriteComment("// ScriptVars");
	f->WriteInt(meta_data.ScriptVar.num);
	foreach(float v, meta_data.ScriptVar)
		f->WriteFloat(v);
	f->WriteComment("// Sun");
	f->WriteBool(meta_data.SunEnabled);
	write_color_3i(f, meta_data.SunAmbient);
	write_color_3i(f, meta_data.SunDiffuse);
	write_color_3i(f, meta_data.SunSpecular);
	f->WriteFloat(meta_data.SunAng.x);
	f->WriteFloat(meta_data.SunAng.y);
	f->WriteComment("// Ambient");
	write_color_3i(f, meta_data.Ambient);
	f->WriteComment("// Physics");
	f->WriteBool(meta_data.PhysicsEnabled);
	f->WriteInt(0);
	f->WriteComment("#");

	delete(f);

	ed->setMessage(_("Gespeichert!"));
	action_manager->markCurrentAsSave();
	return true;
}



bool DataWorld::load(const string & _filename, bool deep)
{
	msg_db_f("World.Load", 1);
	bool Error=false;
	int ffv;

	reset();

	filename = _filename;
	if (this == mode_world->data)
		ed->makeDirs(filename);

	CFile *f = FileOpen(filename);
	if (!f)
		return false;
	file_time = f->GetDateModification().time;

	ffv = f->ReadFileFormatVersion();

	if ((ffv==10)||(ffv==9)){ // new format
		// Terrains
		int n = f->ReadIntC();
		for (int i=0;i<n;i++){
			WorldTerrain t;
			t.FileName = f->ReadStr();
			f->ReadVector(&t.pos);
			Terrains.add(t);
		}
		// Gravitation
		meta_data.Gravity.x = f->ReadFloatC();
		meta_data.Gravity.y = f->ReadFloat();
		meta_data.Gravity.z = f->ReadFloat();
		// EgoIndex
		EgoIndex = f->ReadIntC();
		// Background
		if (ffv == 9)
			f->ReadBoolC(); // BackGroundColorEnabled
		else
			f->ReadComment();
		read_color_argb(f,meta_data.BackGroundColor);
		if (ffv==9){
			meta_data.SkyBoxFile[0] = f->ReadStr();
		}else{
			int ns=f->ReadInt();
			if (ns > meta_data.SkyBoxFile.num)
				meta_data.SkyBoxFile.resize(ns);
			for (int i=0;i<ns;i++)
				meta_data.SkyBoxFile[i] = f->ReadStr();
		}
		// Fog
		meta_data.FogEnabled = f->ReadBoolC();
		meta_data.FogMode = f->ReadWord();
		meta_data.FogStart = f->ReadFloat();
		meta_data.FogEnd = f->ReadFloat();
		meta_data.FogDensity = f->ReadFloat();
		read_color_argb(f, meta_data.FogColor);
		// Music
		n = f->ReadIntC();
		for (int i=0;i<n;i++)
			meta_data.MusicFile.add(f->ReadStr());
		// Objects
		n = f->ReadIntC();
		for (int i=0;i<n;i++){
			WorldObject o;
			o.FileName = f->ReadStr();
			o.Name = f->ReadStr();
			o.pos.x = f->ReadFloat();
			o.pos.y = f->ReadFloat();
			o.pos.z = f->ReadFloat();
			o.Ang.x = f->ReadFloat();
			o.Ang.y = f->ReadFloat();
			o.Ang.z = f->ReadFloat();
			o.object = NULL;
			o.view_stage = 0;
			o.is_selected = false;
			o.is_special = false;
			Objects.add(o);
		}
		// Scripts
		n = f->ReadIntC();
		for (int i=0;i<n;i++){
			string s = f->ReadStr();
			f->ReadInt();
			meta_data.ScriptFile.add(s);
		}
		// ScriptVars
		n = f->ReadIntC();
		meta_data.ScriptVar.clear();
		for (int i=0;i<n;i++)
			meta_data.ScriptVar.add(f->ReadFloat());
		if (f->ReadStr() != "#"){
			meta_data.SunEnabled = f->ReadBool();
			read_color_3i(f, meta_data.SunAmbient);
			read_color_3i(f, meta_data.SunDiffuse);
			read_color_3i(f, meta_data.SunSpecular);
			meta_data.SunAng.x = f->ReadFloat();
			meta_data.SunAng.y = f->ReadFloat();
			f->ReadComment();
			read_color_3i(f, meta_data.Ambient);
			if (f->ReadStr() != "#"){
				meta_data.PhysicsEnabled = f->ReadBool();
			}
		}

	}else{
		ed->errorBox(format(_("Falsches Datei-Format der Datei '%s': %d (statt %d - %d)"), filename.c_str(), ffv, 8, 10));
		Error = true;
	}
	f->Close();

	delete(f);

	if ((!Error)&&(deep)){
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
	notify("Change");

	return !Error;
}


void DataWorld::MetaData::Reset()
{
	ScriptVar.clear();
	ScriptVar.resize(32);

	FogEnabled = false;
	FogMode = FogExp;
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

	ScriptFile.clear();
	MusicFile.clear();
}


void DataWorld::MetaData::ApplyToDraw()
{
	NixSetFog(FogMode, FogStart, FogEnd, FogDensity, FogColor);
	NixEnableFog(FogEnabled);
	NixSetLightDirectional(ed->multi_view_3d->light, -SunAng.ang2dir(), SunAmbient, SunDiffuse, SunSpecular);
	NixEnableLight(ed->multi_view_3d->light, SunEnabled);
	NixSetAmbientLight(Ambient);
}

void DataWorld::MetaData::DrawBackground()
{
	NixResetToColor(BackGroundColor);
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
	notify("Change");
}


void DataWorld::GetBoundaryBox(vector &min, vector &max)
{
	bool found_any=false;
	msg_db_m("GetBoundaryBox",2);
	foreach(WorldObject &o, Objects)
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
	foreach(WorldTerrain &t, Terrains)
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
	foreach(WorldObject &o, Objects)
		if (o.is_selected)
			n ++;
	return n;
}



int DataWorld::GetSelectedTerrains()
{
	int n = 0;
	foreach(WorldTerrain &t, Terrains)
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
	foreach(WorldTerrain &t, Terrains)
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
	foreach(WorldObject &o, Objects)
		o.is_selected = false;
	foreach(WorldTerrain &t, Terrains)
		t.is_selected = false;
}


void DataWorld::Copy(Array<WorldObject> &objects, Array<WorldTerrain> &terrains)
{
	objects.clear();
	terrains.clear();

	foreach(WorldObject &o, Objects)
		if (o.is_selected)
			objects.add(o);
	foreach(WorldTerrain &t, Terrains)
		if (t.is_selected)
			terrains.add(t);
}

void DataWorld::Paste(Array<WorldObject> &objects, Array<WorldTerrain> &terrains)
{	execute(new ActionWorldPaste(objects, terrains));	}

void DataWorld::DeleteSelection()
{	execute(new ActionWorldDeleteSelection());	}
