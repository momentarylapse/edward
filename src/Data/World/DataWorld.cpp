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
	return false;
}



bool DataWorld::Load(const string & _filename, bool deep)
{
	bool Error=false;
	int ffv;

	Reset();
//	if (deep)
//		ProgressStart(MainWin, _("Lade Welt"));
	msg_write("-----------------------------------------------------------");
	msg_write("|   lade Welt                                             |");

	filename = _filename;
	if (this == mode_world->data)
		ed->MakeDirs(filename);

	CFile *f = OpenFile(filename);
	if (!f){
		msg_write("|   fertig                                                |");
		msg_write("-----------------------------------------------------------");
		return false;
	}
//	FileTime=f->GetDate(FileDateModification).time;

	ffv = f->ReadFileFormatVersion();

	if ((ffv==10)||(ffv==9)){ // neues Format
		// Terrains
		int n = f->ReadIntC();
		msg_write("a");
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
		EgoIndex=f->ReadIntC();
		// Background
		if (ffv==9)
			f->ReadBoolC(); // BackGroudColorEnabled
		else
			f->ReadComment();
		read_color_argb(f,meta_data.BackGroundColor);
		msg_write("b");
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
		meta_data.FogEnabled=f->ReadBoolC();
		meta_data.FogMode=f->ReadWord();
		meta_data.FogStart=f->ReadFloat();
		meta_data.FogEnd=f->ReadFloat();
		meta_data.FogDensity=f->ReadFloat();
		read_color_argb(f,meta_data.FogColor);
		msg_write("c");
		// Music
		n = f->ReadIntC();
		for (int i=0;i<n;i++)
			meta_data.MusicFile.add(f->ReadStr());
		// Objects
		msg_write("d");
		n=f->ReadIntC();
		for (int i=0;i<n;i++){
			ModeWorldObject o;
			o.FileName = f->ReadStr();
			o.Name = f->ReadStr();
			o.pos.x=f->ReadFloat();
			o.pos.y=f->ReadFloat();
			o.pos.z=f->ReadFloat();
			o.Ang.x=f->ReadFloat();
			o.Ang.y=f->ReadFloat();
			o.Ang.z=f->ReadFloat();
			o.object=NULL;
			o.view_stage = 0;
			o.is_selected = false;
			o.is_special = false;
			Object.add(o);
		}
		msg_write("e");
		// Scripts
		n=f->ReadIntC();
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
		msg_write("f");
		// ScriptVars
		n=f->ReadIntC();
		meta_data.ScriptVar.clear();
		for (int i=0;i<n;i++)
			meta_data.ScriptVar.add(f->ReadFloat());
		if (f->ReadStr() != "#"){
			meta_data.SunEnabled=f->ReadBool();
			read_color_3i(f, meta_data.SunAmbient);
			read_color_3i(f, meta_data.SunDiffuse);
			read_color_3i(f, meta_data.SunSpecular);
			meta_data.SunAng.x=f->ReadFloat();
			meta_data.SunAng.y=f->ReadFloat();
			f->ReadComment();
			read_color_3i(f, meta_data.Ambient);
		}
		msg_write("g");

	}else{
		ed->ErrorBox(format(_("Falsches Datei-Format der Datei '%s': %d (statt %d - %d)"), filename.c_str(), ffv, 8, 10));
		Error=true;
	}
	f->Close();

	delete(f);
		msg_write(".8");

	if ((!Error)&&(deep)){
//		ProgressEnd();
		for (int i=0;i<Terrain.num;i++)
			Terrain[i].Load(Terrain[i].pos, MapDir + Terrain[i].FileName + ".map", true);
//		ProgressStart(MainWin, _("Lade Objekte"));
		for (int i=0;i<Object.num;i++){
//			Progress(format(_("%d von %d"), i, Object.num), float(i)/float(Object.num));
			Object[i].object = (CObject*)MetaLoadModel(Object[i].FileName);
			Object[i].object->pos = Object[i].pos;
			Object[i].object->ang = Object[i].Ang;
//			if (Object[i].object)
//				GodRegisterModel(Object[i].object);
		}
//		ProgressEnd();
	}

    msg_write("|   fertig                                                |");
	msg_write("-----------------------------------------------------------");



	// debug...
	/*if (JustCreateLightMap){
		for (int i=0;i<Object.num;i++)
			Object[i].is_selected = true;
		CreateLightMap(true);
	}*/
	ResetHistory();
	Notify("Change");

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
	/*for (int i=0;i<Terrain.num;i++){
		vector min2=Terrain[i].Pos+Terrain[i].Min;
		vector max2=Terrain[i].Pos+Terrain[i].Max;
		if (!found_any){
			min=min2;
			max=max2;
		}
		VecMin(min,min2);
		VecMax(max,max2);
		found_any=true;
	}*/
	if (!found_any){
		min=vector(-100,-100,-100);
		max=vector( 100, 100, 100);
	}
}


