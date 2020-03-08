/*
 * ActionWorldDeleteSelection.cpp
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#include "ActionWorldDeleteSelection.h"
#include "Object/ActionWorldDeleteObject.h"
#include "Terrain/ActionWorldDeleteTerrain.h"

ActionWorldDeleteSelection::ActionWorldDeleteSelection()
{
}

ActionWorldDeleteSelection::~ActionWorldDeleteSelection()
{
}

void *ActionWorldDeleteSelection::compose(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);

	for (int i=w->objects.num-1; i>=0; i--)
		if (w->objects[i].is_selected)
			addSubAction(new ActionWorldDeleteObject(i), d);

	for (int i=w->terrains.num-1; i>=0; i--)
		if (w->terrains[i].is_selected)
			addSubAction(new ActionWorldDeleteTerrain(i), d);

	return NULL;
}

