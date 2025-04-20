//
// Created by Michael Ankele on 2025-04-21.
//

#include "ActionWorldEditTerrainMetaData.h"
#include "../../data/DataWorld.h"
#include "../../data/WorldTerrain.h"
#include <y/world/Terrain.h>

ActionWorldEditTerrainMetaData::ActionWorldEditTerrainMetaData(int _index, const vec3& _pattern) {
	index = _index;
	pattern = _pattern;
}

void* ActionWorldEditTerrainMetaData::execute(Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	std::swap(w->entities[index].terrain.terrain->pattern, pattern);
	w->entities[index].terrain.terrain->update(-1, -1, -1, -1, TerrainUpdateAll);
	return nullptr;
}

void ActionWorldEditTerrainMetaData::undo(Data* d) {
	execute(d);
}



