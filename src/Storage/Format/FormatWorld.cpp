/*
 * FormatWorld.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatWorld.h"
#include "../../Edward.h"
#include "../../x/model_manager.h"
#include "../../x/model.h"
#include "../../x/object.h"
#include "../../x/world.h"
#include "../../meta.h"

FormatWorld::FormatWorld() : TypedFormat<DataWorld>(FD_WORLD, "world", _("World"), Flag::CANONICAL_READ_WRITE) {
}

void FormatWorld::_load(const string &filename, DataWorld *data, bool deep) {


	int ffv;

	data->reset();

	File *f = NULL;

	try{

		f = FileOpenText(filename);
		data->file_time = f->GetDateModification().time;

	ffv = f->ReadFileFormatVersion();

	if ((ffv==10) or (ffv==9)){ // new format
		// Terrains
		f->read_comment();
		int n = f->read_int();
		for (int i=0;i<n;i++){
			WorldTerrain t;
			t.FileName = f->read_str();
			f->read_vector(&t.pos);
			data->Terrains.add(t);
		}
		// Gravitation
		f->read_comment();
		data->meta_data.Gravity.x = f->read_float();
		data->meta_data.Gravity.y = f->read_float();
		data->meta_data.Gravity.z = f->read_float();
		// EgoIndex
		f->read_comment();
		data->EgoIndex = f->read_int();
		// Background
		f->read_comment();
		if (ffv == 9)
			f->read_bool(); // BackGroundColorEnabled
		read_color_argb(f, data->meta_data.BackGroundColor);
		if (ffv==9){
			data->meta_data.SkyBoxFile[0] = f->read_str();
		}else{
			int ns=f->read_int();
			if (ns > data->meta_data.SkyBoxFile.num)
				data->meta_data.SkyBoxFile.resize(ns);
			for (int i=0;i<ns;i++)
				data->meta_data.SkyBoxFile[i] = f->read_str();
		}
		// Fog
		f->read_comment();
		data->meta_data.FogEnabled = f->read_bool();
		data->meta_data.FogMode = f->read_word();
		data->meta_data.FogStart = f->read_float();
		data->meta_data.FogEnd = f->read_float();
		data->meta_data.FogDensity = f->read_float();
		read_color_argb(f, data->meta_data.FogColor);
		// Music
		f->read_comment();
		n = f->read_int();
		for (int i=0;i<n;i++)
			data->meta_data.MusicFile.add(f->read_str());
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
			data->Objects.add(o);
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
			data->meta_data.scripts.add(s);
		}
		// ScriptVars
		f->read_comment();
		n = f->read_int();
		for (int i=0;i<n;i++)
			f->read_float();
		if (f->read_str() != "#"){
			data->meta_data.SunEnabled = f->read_bool();
			read_color_3i(f, data->meta_data.SunAmbient);
			read_color_3i(f, data->meta_data.SunDiffuse);
			read_color_3i(f, data->meta_data.SunSpecular);
			data->meta_data.SunAng.x = f->read_float();
			data->meta_data.SunAng.y = f->read_float();
			f->read_comment();
			read_color_3i(f, data->meta_data.Ambient);
			if (f->read_str() != "#"){
				data->meta_data.PhysicsEnabled = f->read_bool();
			}
		}

	}else{
		throw Exception(format(_("File %s has a wrong file format: %d (expected: %d - %d)!"), filename.c_str(), ffv, 8, 10));
	}
	FileClose(f);

	if (deep){
		for (int i=0;i<data->Terrains.num;i++){
			ed->progress->set(_("Terrains"), (float)i / (float)data->Terrains.num / 2.0f);
			data->Terrains[i].Load(data->Terrains[i].pos, MapDir + data->Terrains[i].FileName + ".map", true);
		}
		for (int i=0;i<data->Objects.num;i++){
			//ed->progress->set(format(_("Object %d / %d"), i, data->Objects.num), (float)i / (float)data->Objects.num / 2.0f + 0.5f);
			data->Objects[i].object = (Object*)LoadModel(data->Objects[i].FileName);
			data->Objects[i].object->pos = data->Objects[i].pos;
			data->Objects[i].object->ang = quaternion::rotation_v(data->Objects[i].Ang);
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

	}catch(Exception &e){
		FileClose(f);
		throw e;
	}
}

void FormatWorld::_save(const string &filename, DataWorld *data) {

/*	if (!SaveTerrains())
		return;*/
	File *f = FileCreateText(filename);
	f->float_decimals = 6;

	f->WriteFileFormatVersion(false, 10);
	f->write_comment("// Terrains");
	f->write_int(data->Terrains.num);
	for (auto &t: data->Terrains) {
		f->write_str(t.FileName);
		f->write_float(t.pos.x);
		f->write_float(t.pos.y);
		f->write_float(t.pos.z);
	}
	f->write_comment("// Gravitation");
	f->write_float(data->meta_data.Gravity.x);
	f->write_float(data->meta_data.Gravity.y);
	f->write_float(data->meta_data.Gravity.z);
	f->write_comment("// EgoIndex");
	f->write_int(data->EgoIndex);
	f->write_comment("// Background");
	write_color_argb(f, data->meta_data.BackGroundColor);
	int ns = 0;
	foreachi(string &sb, data->meta_data.SkyBoxFile, i)
		if (sb.num > 0)
			ns = i + 1;
	f->write_int(ns);
	for (int i=0;i<ns;i++)
		f->write_str(data->meta_data.SkyBoxFile[i]);
	f->write_comment("// Fog");
	f->write_bool(data->meta_data.FogEnabled);
	f->write_word(data->meta_data.FogMode);
	f->write_float(data->meta_data.FogStart);
	f->write_float(data->meta_data.FogEnd);
	f->write_float(data->meta_data.FogDensity);
	write_color_argb(f, data->meta_data.FogColor);
	f->write_comment("// Music");
	f->write_int(data->meta_data.MusicFile.num);
	for (string &m: data->meta_data.MusicFile)
		f->write_str(m);
	f->write_comment("// Objects");
	f->write_int(data->Objects.num);
	for (auto &o: data->Objects) {
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
	f->write_int(data->meta_data.scripts.num);
	for (auto &s: data->meta_data.scripts) {
		f->write_str(s.filename);
		f->write_int(s.variables.num);
		for (auto &v: s.variables) {
			f->write_str(v.name);
			f->write_str(v.value);
		}
	}
	f->write_comment("// ScriptVars");
	f->write_int(0);
	f->write_comment("// Sun");
	f->write_bool(data->meta_data.SunEnabled);
	write_color_3i(f, data->meta_data.SunAmbient);
	write_color_3i(f, data->meta_data.SunDiffuse);
	write_color_3i(f, data->meta_data.SunSpecular);
	f->write_float(data->meta_data.SunAng.x);
	f->write_float(data->meta_data.SunAng.y);
	f->write_comment("// Ambient");
	write_color_3i(f, data->meta_data.Ambient);
	f->write_comment("// Physics");
	f->write_bool(data->meta_data.PhysicsEnabled);
	f->write_int(0);
	f->write_comment("#");

	delete(f);

}
