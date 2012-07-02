/*
 * DataWorld.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "DataWorld.h"
#include "../../Mode/World/ModeWorld.h"
#include "../../Edward.h"


void ModeWorldObject::UpdateData()
{
	if (!object)
		return;
	object->pos = pos;
	object->ang = Ang;
	object->UpdateMatrix();
}

bool ModeWorldTerrain::Load(const vector &_pos, const string &filename, bool deep)
{
	msg_db_r("Terrain.LoadFromFile", 1);
	view_stage = 0;
	is_selected = false;
	is_special = false;

	if (deep)
		ed->MakeDirs(filename);
	FileName = filename.substr(MapDir.num, -1);
	FileName.resize(FileName.num - 4);

	terrain = new CTerrain();
	bool Error = !terrain->Load(FileName, _pos, deep);

	if (Error){
		delete(terrain);
		terrain = NULL;
	}

	return !Error;
}

void ModeWorldTerrain::UpdateData()
{
	if (!terrain)
		return;
	terrain->pos = pos;
}

DataWorld::DataWorld()
{
}

DataWorld::~DataWorld()
{
}

bool DataWorld::Save(const string & _filename)
{
/*	if (!SaveTerrains())
		return;*/
	filename = _filename;
	ed->MakeDirs(filename);
	CFile *f = CreateFile(filename);
	f->FloatDecimals = 6;

	f->WriteFileFormatVersion(false, 10);
	f->WriteComment("// Terrains");
	f->WriteInt(Terrain.num);
	foreach(Terrain, t){
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
	foreachi(meta_data.SkyBoxFile, sb, i)
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
	foreach(meta_data.MusicFile, m)
		f->WriteStr(m);
	f->WriteComment("// Objects");
	f->WriteInt(Object.num);
	foreach(Object, o){
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
	f->WriteInt(meta_data.Script.num);
	foreach(meta_data.Script, s){
		f->WriteStr(s.Filename);
		f->WriteInt(s.Rule.num);
		foreach(s.Rule, r){
			f->WriteStr(r.Function);
			f->WriteInt(r.Location);
		}
	}
	f->WriteComment("// ScriptVars");
	f->WriteInt(meta_data.ScriptVar.num);
	foreach(meta_data.ScriptVar, v)
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
	f->WriteComment("#");

	delete(f);

	ed->SetMessage(_("Gespeichert!"));
	action_manager->MarkCurrentAsSave();
	return true;
}



bool DataWorld::Load(const string & _filename, bool deep)
{
	msg_db_r("World.Load", 1);
	bool Error=false;
	int ffv;

	Reset();

	filename = _filename;
	if (this == mode_world->data)
		ed->MakeDirs(filename);

	CFile *f = OpenFile(filename);
	if (!f){
		msg_db_l(1);
		return false;
	}
//	FileTime=f->GetDate(FileDateModification).time;

	ffv = f->ReadFileFormatVersion();

	if ((ffv==10)||(ffv==9)){ // new format
		// Terrains
		int n = f->ReadIntC();
		for (int i=0;i<n;i++){
			ModeWorldTerrain t;
			t.FileName = f->ReadStr();
			f->ReadVector(&t.pos);
			Terrain.add(t);
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
			ModeWorldObject o;
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
			Object.add(o);
		}
		// Scripts
		n = f->ReadIntC();
		for (int i=0;i<n;i++){
			ModeWorldScript s;
			s.Filename = f->ReadStr();
			int NumRules = f->ReadInt();
			for (int n=0;n<NumRules;n++){
				ModeWorldScriptRule r;
				r.Function = f->ReadStr();
				r.Location = f->ReadInt();
				s.Rule.add(r);
			}
			meta_data.Script.add(s);
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
		}

	}else{
		ed->ErrorBox(format(_("Falsches Datei-Format der Datei '%s': %d (statt %d - %d)"), filename.c_str(), ffv, 8, 10));
		Error = true;
	}
	f->Close();

	delete(f);

	if ((!Error)&&(deep)){
		for (int i=0;i<Terrain.num;i++){
			ed->progress->Set(_("Terrains"), (float)i / (float)Terrain.num / 2.0f);
			Terrain[i].Load(Terrain[i].pos, MapDir + Terrain[i].FileName + ".map", true);
		}
		for (int i=0;i<Object.num;i++){
			ed->progress->Set(format(_("Objekt %d von %d"), i, Object.num), (float)i / (float)Object.num / 2.0f + 0.5f);
			Object[i].object = (CObject*)MetaLoadModel(Object[i].FileName);
			Object[i].object->pos = Object[i].pos;
			Object[i].object->ang = Object[i].Ang;
//			if (Object[i].object)
//				GodRegisterModel(Object[i].object);
		}
	}



	// debug...
	/*if (JustCreateLightMap){
		for (int i=0;i<Object.num;i++)
			Object[i].is_selected = true;
		CreateLightMap(true);
	}*/
	ResetHistory();
	Notify("Change");

	msg_db_l(1);
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

	BackGroundColor = Black;

	PhysicsEnabled = true;
	Gravity = vector(0, -981, 0);

	Script.clear();
	MusicFile.clear();
}


void DataWorld::MetaData::ApplyToDraw()
{
	NixSetFog(FogMode, FogStart, FogEnd, FogDensity, FogColor);
	NixEnableFog(FogEnabled);
	NixSetLightDirectional(ed->multi_view_3d->light, VecAng2Dir(SunAng), SunAmbient, SunDiffuse, SunSpecular);
	NixEnableLight(ed->multi_view_3d->light, SunEnabled);
	NixSetAmbientLight(Ambient);
}

void DataWorld::MetaData::DrawBackground()
{
	NixSetZ(false,false);
	NixSetWire(false);
	NixDraw2D(-1, BackGroundColor, r_id, NixTargetRect, 0);
	NixSetWire(ed->multi_view_3d->wire_mode);
	NixSetZ(true,true);
}


void DataWorld::Reset()
{
	filename = "";

	// delete old data...
	for (int i=0;i<Object.num;i++)
		if (Object[i].object)
			MetaDeleteModel(Object[i].object);

	Object.clear();
	Terrain.clear();

	EgoIndex = -1;

	meta_data.Reset();

	ResetHistory();
	Notify("Change");
}


void DataWorld::GetBoundaryBox(vector &min, vector &max)
{
	bool found_any=false;
	msg_db_m("GetBoundaryBox",2);
	foreach(Object, o)
		if (o.object){
			vector min2 = o.pos - vector(1,1,1) * o.object->radius;
			vector max2 = o.pos + vector(1,1,1) * o.object->radius;
			if (!found_any){
				min = min2;
				max = max2;
			}
			VecMin(min, min2);
			VecMax(max, max2);
			found_any = true; //|=(min2!=max2);
		}
	foreach(Terrain, t)
		if (t.terrain){
			vector min2 = t.terrain->min;
			vector max2 = t.terrain->max;
			if (!found_any){
				min = min2;
				max = max2;
			}
			VecMin(min, min2);
			VecMax(max, max2);
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
	foreach(Object, o)
		if (o.is_selected)
			n ++;
	return n;
}



int DataWorld::GetSelectedTerrains()
{
	int n = 0;
	foreach(Terrain, t)
		if (t.is_selected)
			n ++;
	return n;
}


void DataWorld::UpdateData()
{
	foreachi(Object, o, i){
		o.UpdateData();
		o.is_special = (i == EgoIndex);
	}
	foreach(Terrain, t)
		t.UpdateData();
}




