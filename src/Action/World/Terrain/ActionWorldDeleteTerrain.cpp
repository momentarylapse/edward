/*
 * ActionWorldDeleteTerrain.cpp
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#include "ActionWorldDeleteTerrain.h"
#include <assert.h>

ActionWorldDeleteTerrain::ActionWorldDeleteTerrain(int _index)
{
	index = _index;
}

ActionWorldDeleteTerrain::~ActionWorldDeleteTerrain()
{
}

void* ActionWorldDeleteTerrain::execute(Data* d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	assert(index >= 0);
	assert(index < w->Terrains.num);

	terrain = w->Terrains[index];
	w->Terrains.erase(index);

	return NULL;
}

void ActionWorldDeleteTerrain::undo(Data* d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);

	terrain.is_selected = false;
	w->Terrains.insert(terrain, index);
}
