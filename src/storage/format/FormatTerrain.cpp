/*
 * FormatTerrain.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatTerrain.h"
#include "../../EdwardWindow.h"
#include "../../data/world/WorldTerrain.h"

FormatTerrain::FormatTerrain(Session *s) : TypedFormat<WorldTerrain>(s, FD_TERRAIN, "map", _("Terrain"), Flag::CANONICAL_READ_WRITE) {
}

void FormatTerrain::_load(const Path &filename, WorldTerrain *data, bool deep) {
	data->load(session, filename, deep);
}

void FormatTerrain::_save(const Path &filename, WorldTerrain *data) {
	data->save(filename);
}
