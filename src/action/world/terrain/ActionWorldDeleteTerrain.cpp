/*
 * ActionWorldDeleteTerrain.cpp
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#include "ActionWorldDeleteTerrain.h"
#include "../../../data/world/DataWorld.h"
#include <assert.h>

ActionWorldDeleteTerrain::ActionWorldDeleteTerrain(int _index) {
	index = _index;
}

void* ActionWorldDeleteTerrain::execute(Data* d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	assert(index >= 0);
	assert(index < w->terrains.num);

	terrain = w->terrains[index];
	w->terrains.erase(index);

	return NULL;
}

void ActionWorldDeleteTerrain::undo(Data* d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);

	terrain.is_selected = false;
	w->terrains.insert(terrain, index);
}
