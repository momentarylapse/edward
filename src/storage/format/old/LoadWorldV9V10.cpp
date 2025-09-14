/*
 * LoadWorldV9V10.cpp
 *
 *  Created on: 10 Sept 2023
 *      Author: michi
 */


#include "../FormatWorld.h"
#include "../../../mode_world/data/DataWorld.h"
#include "../../../mode_world/data/WorldLink.h"
#include "../../../mode_world/data/WorldObject.h"
#include "../../../mode_world/data/WorldTerrain.h"
//#include "../../../EdwardWindow.h"
#include <Session.h>
#include <y/world/Model.h>
#include <y/world/World.h>
#include <y/EngineData.h>
#include <y/world/ModelManager.h>
#include <y/world/components/Skeleton.h>
#include "../../../lib/os/date.h"
#include "../../../lib/os/file.h"
#include "../../../lib/os/filesystem.h"
#include "../../../lib/os/formatter.h"



void FormatWorld::_load_old(LegacyFile& lf, DataWorld *data, bool deep) {

	// TODO read into LevelData...
#if 0
	os::fs::FileStream *f = lf.f;

	try{

		data->file_time = os::fs::mtime(lf.filename).time;

	if ((lf.ffv == 10) or (lf.ffv == 9)){ // new format
		// Terrains
		f->read_comment();
		int n = f->read_int();
		for (int i=0;i<n;i++){
			WorldEntity t;
			t.basic_type = MultiViewType::WORLD_TERRAIN;
			t.terrain.filename = f->read_str();
			f->read_vector(&t.pos);
			data->entities.add(t);
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
		if (lf.ffv == 9)
			f->read_bool(); // BackGroundColorEnabled
		read_color_argb(f, data->meta_data.background_color);
		if (lf.ffv==9){
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
		data->meta_data.fog.mode = (ygfx::FogMode)f->read_word();
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
			WorldEntity o;
			o.basic_type = MultiViewType::WORLD_OBJECT;
			o.object.filename = f->read_str();
			o.object.name = f->read_str();
			o.pos.x = f->read_float();
			o.pos.y = f->read_float();
			o.pos.z = f->read_float();
			o.ang.x = f->read_float();
			o.ang.y = f->read_float();
			o.ang.z = f->read_float();
			o.object.object = nullptr;
			o.view_stage = 0;
			data->entities.add(o);
		}
		// Scripts
		f->read_comment();
		n = f->read_int();
		for (int i=0;i<n;i++){
			ScriptInstanceData s;
			s.filename = f->read_str();
			int nv = f->read_int();
			for (int j=0; j<nv; j++){
				WorldScriptVariable var;
				var.name = f->read_str();
				var.value = f->read_str();
				s.variables.add(var);
			}
			data->meta_data.systems.add(s);
		}
		// ScriptVars
		f->read_comment();
		n = f->read_int();
		for (int i=0;i<n;i++)
			f->read_float();
		if (f->read_str() != "#"){

			WorldEntity sun;
			sun.basic_type = MultiViewType::WORLD_LIGHT;
			sun.light.enabled = f->read_bool();
			color am, am2, di, sp;
			read_color_3i(f, am);
			read_color_3i(f, di);
			read_color_3i(f, sp);
			float ang_x = f->read_float();
			float ang_y = f->read_float();
			// fix for old definition
			sun.ang = quaternion::rotation((-vec3(ang_x, ang_y, 0).ang2dir()).dir2ang());
			f->read_comment();
			read_color_3i(f, am2);
			sun.light.col = (am + am2) * 2 + di;
			sun.light.harshness = di.r / sun.light.col.r;
			data->entities.add(sun);
			if (f->read_str() != "#"){
				data->meta_data.physics_enabled = f->read_bool();
			}
		}

	}else{
		throw Exception(format("File '%s' has an unhandled legacy file format: %d (expected: %d - %d)!", lf.filename.c_str(), lf.ffv, 8, 10));
	}
	delete f;

	}catch(Exception &e){
		delete f;
		throw e;
	}
#endif
}

