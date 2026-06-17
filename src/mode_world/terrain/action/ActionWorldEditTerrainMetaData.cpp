//
// Created by Michael Ankele on 2025-04-21.
//

#include "ActionWorldEditTerrainMetaData.h"
#include "../../data/DataWorld.h"
#include <y/world/Terrain.h>
#include <ecs/Entity.h>

ActionWorldEditTerrainMetaData::ActionWorldEditTerrainMetaData(Terrain* t, const vec3& _pattern, const vec3 _texture_scale[8]) {
	terrain = t;
	pattern = _pattern;
	for (int i=0; i<8; i++)
		texture_scale[i] = _texture_scale[i];
}

void* ActionWorldEditTerrainMetaData::execute(history::Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	std::swap(terrain->pattern, pattern);
	for (int i=0; i<8; i++)
		std::swap(terrain->texture_scale[i], texture_scale[i]);
	terrain->update(-1, -1, -1, -1, TerrainUpdateAll);

	w->out_terrain_changed(terrain);
	return nullptr;
}

void ActionWorldEditTerrainMetaData::undo(history::Data* d) {
	execute(d);
}



