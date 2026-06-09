/*
 * LoadWorldV9V10.cpp
 *
 *  Created on: 10 Sept 2023
 *      Author: michi
 */


#include "../FormatWorld.h"
#include "../../../mode_world/data/DataWorld.h"
#include "../../../mode_world/data/WorldLink.h"
//#include "../../../mode_world/data/WorldObject.h"
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
#include "lib/os/msg.h"
#include "lib/yrenderer/scene/Light.h"


void FormatWorld::_load_old(LegacyFile& lf, LevelData& ld) {
	msg_write("LOAD V10...");

	// TODO read into LevelData...
#if 1
	os::fs::FileStream *f = lf.f;

	try{

	if ((lf.ffv == 10) or (lf.ffv == 9)){ // new format
		// Terrains
		f->read_comment();
		int n = f->read_int();
		for (int i=0;i<n;i++){
			LevelData::Entity o;
			ecs::InstanceData t = {"TerrainRef"};
			t.set("terrain", f->read_str());
			//t.set("material", ...);
			f->read_vector(&o.pos);
			o.ang = quaternion::ID;
			o.components.add(t);
			o.components.append(LevelData::auto_terrain_components());
			ld.entities.add(o);
		}
		// Gravitation
		f->read_comment();
		ld.gravity.x = f->read_float();
		ld.gravity.y = f->read_float();
		ld.gravity.z = f->read_float();
		// EgoIndex
		f->read_comment();
		int EgoIndex = f->read_int();
		// Background
		f->read_comment();
		if (lf.ffv == 9)
			f->read_bool(); // BackGroundColorEnabled
		read_color_argb(f, ld.background_color);
		if (lf.ffv == 9) {
			ld.skybox_filename.add(f->read_str());
			ld.skybox_ang.add(v_0);
		}else{
			int ns = f->read_int();
			for (int i=0; i<ns; i++) {
				ld.skybox_filename.add(f->read_str());
				ld.skybox_ang.add(v_0);
			}
		}
		// Fog
		f->read_comment();
		ld.fog.enabled = f->read_bool();
		ld.fog.mode = (int)f->read_word();
		ld.fog.start = f->read_float();
		ld.fog.end = f->read_float();
		ld.fog.distance = 1.0f / f->read_float();
		read_color_argb(f, ld.fog._color);
		// Music
		f->read_comment();
		n = f->read_int();
		for (int i=0; i<n; i++)
			f->read_str();

		// Objects
		f->read_comment();
		n = f->read_int();
		for (int i=0; i<n; i++) {
			LevelData::Entity o;
			ecs::InstanceData t = {"TemplateRef"};
			t.set("template", f->read_str());
			f->read_str(); // name
			o.pos.x = f->read_float();
			o.pos.y = f->read_float();
			o.pos.z = f->read_float();
			vec3 ang;
			ang.x = f->read_float();
			ang.y = f->read_float();
			ang.z = f->read_float();
			o.ang = quaternion::rotation_v(ang);
			o.components.add(t);
			if (i == EgoIndex)
				o.components.add({"EgoMarker", "", {}});
			ld.entities.add(o);
		}
		// Scripts
		f->read_comment();
		n = f->read_int();
		for (int i=0;i<n;i++){
			ecs::InstanceData s;
			s.filename = f->read_str();
			int nv = f->read_int();
			for (int j=0; j<nv; j++){
				ecs::InstanceDataVariable var;
				var.name = f->read_str();
				var.value = Any::parse(f->read_str());
				s.variables.add(var);
			}
			ld.systems.add(s);
		}
		// ScriptVars
		f->read_comment();
		n = f->read_int();
		for (int i=0;i<n;i++)
			f->read_float();
		if (f->read_str() != "#"){


			LevelData::Entity sun;
			ecs::InstanceData l = {"Light"};
			l.set("type", (int)yrenderer::LightType::DIRECTIONAL);
			l.set("theta", -1.0);
			l.set("power", 1.0);
			l.set("allow_shadow", true);
			l.set("enabled", f->read_bool());
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
			color col = (am + am2) * 2 + di;
			l.set("color", str(col));
			l.set("harshness", di.r / col.r);
			sun.components.add(l);
			ld.entities.add(sun);

			if (f->read_str() != "#"){
				ld.physics_enabled = f->read_bool();
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

