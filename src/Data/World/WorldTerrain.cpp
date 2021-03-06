/*
 * WorldTerrain.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */


#include "WorldTerrain.h"
#include "../../y/EngineData.h"
#include "../../y/Terrain.h"
#include "../../lib/file/file.h"


bool WorldTerrain::load(const vector &_pos, const Path &_filename, bool deep) {

	filename = _filename.relative_to(engine.map_dir).no_ext();

	terrain = new Terrain();
	bool Error = !terrain->load(filename, _pos, deep);

	if (Error) {
		delete(terrain);
		terrain = NULL;
	}

	return !Error;
}

bool WorldTerrain::save(const Path &_filename) {
	filename = _filename.relative_to(engine.map_dir).no_ext();


	File *f = NULL;

	try{
		f = FileCreate(_filename);

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
		f->write_str(terrain->texture_file[i].str());
		f->write_float(terrain->texture_scale[i].x);
		f->write_float(terrain->texture_scale[i].z);
	}
	f->write_str(terrain->material_file.str());

	// height
	for (int x=0;x<terrain->num_x+1;x++)
		for (int z=0;z<terrain->num_z+1;z++)
			f->write_float(terrain->height[x*(terrain->num_z+1) + z]);

	FileClose(f);

	}catch(Exception &e){}

	return true;
}

void WorldTerrain::update_data() {
	if (!terrain)
		return;
	terrain->pos = pos;
}

