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

	for (int i=w->Objects.num-1; i>=0; i--)
		if (w->Objects[i].is_selected)
			AddSubAction(new ActionWorldDeleteObject(i), d);

	for (int i=w->Terrains.num-1; i>=0; i--)
		if (w->Terrains[i].is_selected)
			AddSubAction(new ActionWorldDeleteTerrain(i), d);

	return NULL;
}

