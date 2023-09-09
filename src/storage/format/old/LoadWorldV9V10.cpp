/*
 * LoadWorldV9V10.cpp
 *
 *  Created on: 10 Sept 2023
 *      Author: michi
 */


#include "../FormatWorld.h"
#include "../../../data/world/DataWorld.h"
#include "../../../data/world/WorldCamera.h"
#include "../../../data/world/WorldLight.h"
#include "../../../data/world/WorldLink.h"
#include "../../../data/world/WorldObject.h"
#include "../../../data/world/WorldTerrain.h"
#include "../../../Edward.h"
#include "../../../y/Model.h"
#include "../../../y/Object.h"
#include "../../../y/World.h"
#include "../../../y/EngineData.h"
#include "../../../y/ModelManager.h"
#include "../../../y/components/Skeleton.h"
#include "../../../lib/os/date.h"
#include "../../../lib/os/file.h"
#include "../../../lib/os/filesystem.h"
#include "../../../lib/os/formatter.h"



void FormatWorld::_load_old(const Path &filename, DataWorld *data, bool deep) {

	TextLinesFormatter *f = nullptr;
	int ffv;

	try{

		f = new TextLinesFormatter(os::fs::open(filename, "rt"));
		data->file_time = os::fs::mtime(filename).time;

	//ffv = f->ReadFileFormatVersion();
		f->read(1);
		ffv = f->read_word();

	if ((ffv==10) or (ffv==9)){ // new format
		// Terrains
		f->read_comment();
		int n = f->read_int();
		for (int i=0;i<n;i++){
			WorldTerrain t;
			t.filename = f->read_str();
			f->read_vector(&t.pos);
			data->terrains.add(t);
		}
		// Gravitation
		f->read_comment();
		data->meta_data.gravity.x = f->read_float();
		data->meta_data.gravity.y = f->read_float();
		data->meta_data.gravity.z = f->read_float();
		// EgoIndex
		f->read_comment();
		data->EgoIndex = f->read_int();
		// Background
		f->read_comment();
		if (ffv == 9)
			f->read_bool(); // BackGroundColorEnabled
		read_color_argb(f, data->meta_data.background_color);
		if (ffv==9){
			data->meta_data.skybox_files[0] = f->read_str();
		}else{
			int ns=f->read_int();
			if (ns > data->meta_data.skybox_files.num)
				data->meta_data.skybox_files.resize(ns);
			for (int i=0;i<ns;i++)
				data->meta_data.skybox_files[i] = f->read_str();
		}
		// Fog
		f->read_comment();
		data->meta_data.fog.enabled = f->read_bool();
		data->meta_data.fog.mode = (nix::FogMode)f->read_word();
		data->meta_data.fog.start = f->read_float();
		data->meta_data.fog.end = f->read_float();
		data->meta_data.fog.density = f->read_float();
		read_color_argb(f, data->meta_data.fog.col);
		// Music
		f->read_comment();
		n = f->read_int();
		for (int i=0;i<n;i++)
			data->meta_data.music_files.add(f->read_str());
		// Objects
		f->read_comment();
		n = f->read_int();
		for (int i=0;i<n;i++){
			WorldObject o;
			o.filename = f->read_str();
			o.name = f->read_str();
			o.pos.x = f->read_float();
			o.pos.y = f->read_float();
			o.pos.z = f->read_float();
			o.ang.x = f->read_float();
			o.ang.y = f->read_float();
			o.ang.z = f->read_float();
			o.object = NULL;
			o.view_stage = 0;
			o.is_selected = false;
			o.is_special = false;
			data->objects.add(o);
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
			data->lights[0].enabled = f->read_bool();
			color am, am2, di, sp;
			read_color_3i(f, am);
			read_color_3i(f, di);
			read_color_3i(f, sp);
			data->lights[0].ang.x = f->read_float();
			data->lights[0].ang.y = f->read_float();
			// fix for old definition
			data->lights[0].ang = (-data->lights[0].ang.ang2dir()).dir2ang();
			f->read_comment();
			read_color_3i(f, am2);
			data->lights[0].col = (am + am2) * 2 + di;
			data->lights[0].harshness = di.r / data->lights[0].col.r;
			if (f->read_str() != "#"){
				data->meta_data.physics_enabled = f->read_bool();
			}
		}

	}else{
		throw Exception(format(_("File '%s' has an unhandled legacy file format: %d (expected: %d - %d)!"), filename.c_str(), ffv, 8, 10));
	}
	delete f;

	}catch(Exception &e){
		delete f;
		throw e;
	}
}

