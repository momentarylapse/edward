/*
 * WorldTerrain.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */


#include "WorldTerrain.h"
#include "../../meta.h"
#include "../../x/terrain.h"


bool WorldTerrain::load(const vector &_pos, const string &_filename, bool deep) {
	view_stage = 0;
	is_selected = false;
	is_special = false;

	filename = _filename.substr(engine.map_dir.num, -1);
	filename.resize(filename.num - 4);

	terrain = new Terrain();
	bool Error = !terrain->load(filename, _pos, deep);

	if (Error) {
		delete(terrain);
		terrain = NULL;
	}

	return !Error;
}

bool WorldTerrain::save(const string &_filename) {
	filename = _filename.substr(engine.map_dir.num, -1);
	filename.resize(filename.num - 4);


	File *f = NULL;

	try{
		f = FileCreate(filename);

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
		f->write_str(terrain->texture_file[i]);
		f->write_float(terrain->texture_scale[i].x);
		f->write_float(terrain->texture_scale[i].z);
	}
	f->write_str(terrain->material_file);

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

