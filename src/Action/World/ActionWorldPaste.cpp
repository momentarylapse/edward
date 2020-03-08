/*
 * ActionWorldPaste.cpp
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#include "ActionWorldPaste.h"
#include "Object/ActionWorldAddObject.h"
#include "Terrain/ActionWorldAddTerrain.h"
#include "../../Data/World/DataWorld.h"
#include "../../Data/World/WorldObject.h"
#include "../../Data/World/WorldTerrain.h"


ActionWorldPaste::ActionWorldPaste(Array<WorldObject> &_objects, Array<WorldTerrain> &_terrains) :
	objects(_objects),
	terrains(_terrains)
{
}

void *ActionWorldPaste::compose(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	w->ClearSelection();

	for (WorldObject &o: objects)
		addSubAction(new ActionWorldAddObject(o), w);

	for (WorldTerrain &t: terrains)
		addSubAction(new ActionWorldAddTerrain(t.pos, t.filename), w);

	return NULL;
}

