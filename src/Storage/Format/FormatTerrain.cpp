/*
 * FormatTerrain.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatTerrain.h"
#include "../../Edward.h"
#include "../../Data/World/WorldTerrain.h"

FormatTerrain::FormatTerrain() : TypedFormat<WorldTerrain>(FD_TERRAIN, "map", _("Terrain"), Flag::CANONICAL_READ_WRITE) {
}

void FormatTerrain::_load(const Path &filename, WorldTerrain *data, bool deep) {
	data->load(v_0, filename, deep);
}

void FormatTerrain::_save(const Path &filename, WorldTerrain *data) {
	data->save(filename);
}
