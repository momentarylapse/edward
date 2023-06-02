/*
 * FormatTerrain.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatTerrain.h"
#include "../../Edward.h"
#include "../../data/world/WorldTerrain.h"

FormatTerrain::FormatTerrain() : TypedFormat<WorldTerrain>(FD_TERRAIN, "map", _("Terrain"), Flag::CANONICAL_READ_WRITE) {
}

void FormatTerrain::_load(const Path &filename, WorldTerrain *data, bool deep) {
	data->load(filename, deep);
}

void FormatTerrain::_save(const Path &filename, WorldTerrain *data) {
	data->save(filename);
}
