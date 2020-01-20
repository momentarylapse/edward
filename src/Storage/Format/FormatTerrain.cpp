/*
 * FormatTerrain.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatTerrain.h"
#include "../../Edward.h"

FormatTerrain::FormatTerrain() : TypedFormat<WorldTerrain>(FD_TERRAIN, "terrain", _("Terrain"), Flag::CANONICAL_READ_WRITE) {
}

void FormatTerrain::_load(const string &filename, WorldTerrain *data, bool deep) {
	data->Load(v_0, filename, deep);
}

void FormatTerrain::_save(const string &filename, WorldTerrain *data) {
	data->Save(filename);
}
