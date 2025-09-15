//
// Created by Michael Ankele on 2025-04-21.
//

#include "ActionWorldEditTerrainMetaData.h"
#include "../../data/DataWorld.h"
#include <y/world/Terrain.h>
#include <y/Entity.h>

ActionWorldEditTerrainMetaData::ActionWorldEditTerrainMetaData(int _index, const vec3& _pattern) {
	index = _index;
	pattern = _pattern;
}

void* ActionWorldEditTerrainMetaData::execute(Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	auto terrain = w->entity(index)->get_component<TerrainRef>()->terrain;
	std::swap(terrain->pattern, pattern);
	terrain->update(-1, -1, -1, -1, TerrainUpdateAll);
	return nullptr;
}

void ActionWorldEditTerrainMetaData::undo(Data* d) {
	execute(d);
}



