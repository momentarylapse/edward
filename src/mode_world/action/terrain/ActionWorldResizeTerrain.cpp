//
// Created by Michael Ankele on 2025-04-23.
//

#include "ActionWorldResizeTerrain.h"
#include "../../data/DataWorld.h"
#include "../../data/WorldTerrain.h"
#include <y/world/Terrain.h>

ActionWorldResizeTerrain::ActionWorldResizeTerrain(int _index, int _nx, int _nz) {
	index = _index;
	nx = _nx;
	nz = _nz;
	heights.resize((nx + 1) * (nz + 1));
}

void* ActionWorldResizeTerrain::execute(Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	std::swap(w->entities[index].terrain.terrain->num_x, nx);
	std::swap(w->entities[index].terrain.terrain->num_z, nz);
	std::swap(w->entities[index].terrain.terrain->height, heights);
	w->entities[index].terrain.terrain->update(-1, -1, -1, -1, TerrainUpdateAll);
	return nullptr;
}

void ActionWorldResizeTerrain::undo(Data* d) {
	execute(d);
}



