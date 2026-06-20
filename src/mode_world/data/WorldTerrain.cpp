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


WorldTerrain::WorldTerrain() : Data(FD_TERRAIN) {
}

WorldTerrain::~WorldTerrain() = default;

bool WorldTerrain::load(const Path &_filename, bool deep) {
	filename = _filename;

	terrain = new Terrain();
	terrain->filename_rel = filename.relative_to(doc->session->resource_manager->map_dir);
	bool Error = !terrain->reload(doc->session->resource_manager, deep);

	if (Error) {
		delete terrain ;
		terrain = nullptr;
	}

	return !Error;
}

bool WorldTerrain::save(const Path &_filename) {
	filename = _filename;


	os::fs::FileStream *f = nullptr;

	try{
		f = os::fs::open(filename, "wb");

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
	f->write_int(2);
	for (int i=0; i<2; i++) {
		f->write_str(format("rotation=%.2f", terrain->texture_scale[i].y));
		f->write_float(terrain->texture_scale[i].x);
		f->write_float(terrain->texture_scale[i].z);
	}
	f->write_str(""); // material

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


