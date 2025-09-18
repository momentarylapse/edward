/*
 * FormatTerrain.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatTerrain.h"
#include <mode_world/data/WorldTerrain.h>

FormatTerrain::FormatTerrain(Session *s) : TypedFormat<WorldTerrain>(s, FD_TERRAIN, "map", "Terrain", Flag::CANONICAL_READ_WRITE) {
}

void FormatTerrain::_load(const Path &filename, WorldTerrain *data, bool deep) {
	data->load(filename, deep);
}

void FormatTerrain::_save(const Path &filename, WorldTerrain *data) {
	data->save(filename);
}
