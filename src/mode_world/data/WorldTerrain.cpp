/*
 * WorldTerrain.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */


#include "WorldTerrain.h"

#include <helper/ResourceManager.h>
#include <mode_material/data/DataMaterial.h>
#include <storage/Storage.h>

#include "DataWorld.h"
#include <Session.h>
#include <y/EngineData.h>
#include <y/world/Terrain.h>
#include <lib/os/file.h>
#include <lib/os/filesystem.h>
#include <lib/os/formatter.h>
#include <view/DocumentSession.h>


bool WorldTerrain::load(const Path &_filename, bool deep) {

	filename = _filename.relative_to(engine.map_dir).no_ext();

	terrain = new Terrain();
	terrain->filename = filename;
	bool Error = !terrain->reload(doc->session->resource_manager, deep);

	if (Error) {
		delete terrain ;
		terrain = nullptr;
	}

	return !Error;
}

bool WorldTerrain::save(const Path &_filename) {
	filename = _filename.relative_to(engine.map_dir).no_ext();


	os::fs::FileStream *f = nullptr;

	try{
		f = os::fs::open(_filename, "wb");

		//f->WriteFileFormatVersion(true, 4);
	f->write_byte('b');
	f->write_word(4);
	f->write_byte(0);

	// Metrics
	f->write_comment("// Metrics");
	f->write_int(terrain->num_x);
	f->write_int(terrain->num_z);
	f->write_float(terrain->pattern.x);
	f->write_float(terrain->pattern.z);

	// Textures
	f->write_comment("// Textures");
	f->write_int(0);
	f->write_str("");

	// height
	for (int x=0;x<terrain->num_x+1;x++)
		for (int z=0;z<terrain->num_z+1;z++)
			f->write_float(terrain->height[x*(terrain->num_z+1) + z]);

	delete f;

	}catch(Exception &e){}

	return true;
}

void WorldTerrain::update_data() {
	if (!terrain)
		return;
	//terrain->pos = pos;
}




Box WorldTerrain::bounding_box() const {
	return {v_0, v_0};//pos, pos};
}


