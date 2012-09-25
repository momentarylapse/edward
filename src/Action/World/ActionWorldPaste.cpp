/*
 * ActionWorldPaste.cpp
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#include "ActionWorldPaste.h"
#include "ActionWorldAddObject.h"
#include "ActionWorldAddTerrain.h"
#include "../../Data/World/DataWorld.h"


ActionWorldPaste::ActionWorldPaste(Array<WorldObject> &_objects, Array<WorldTerrain> &_terrains) :
	objects(_objects),
	terrains(_terrains)
{
}

void *ActionWorldPaste::compose(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	w->ClearSelection();

	foreach(WorldObject &o, objects)
		AddSubAction(new ActionWorldAddObject(o.FileName, o.pos, o.Ang), w);

	foreach(WorldTerrain &t, terrains)
		AddSubAction(new ActionWorldAddTerrain(t.pos, t.FileName), w);

	return NULL;
}

