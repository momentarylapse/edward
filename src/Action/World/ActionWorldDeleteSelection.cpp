/*
 * ActionWorldDeleteSelection.cpp
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#include "ActionWorldDeleteSelection.h"
#include "Object/ActionWorldDeleteObject.h"
#include "Terrain/ActionWorldDeleteTerrain.h"
#include "Link/ActionWorldDeleteLink.h"
#include "Light/ActionWorldDeleteLight.h"

ActionWorldDeleteSelection::ActionWorldDeleteSelection() {
}

void *ActionWorldDeleteSelection::compose(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);

	for (int i=w->objects.num-1; i>=0; i--)
		if (w->objects[i].is_selected)
			addSubAction(new ActionWorldDeleteObject(i), d);

	for (int i=w->terrains.num-1; i>=0; i--)
		if (w->terrains[i].is_selected)
			addSubAction(new ActionWorldDeleteTerrain(i), d);

	for (int i=w->links.num-1; i>=0; i--)
		if (w->links[i].is_selected)
			addSubAction(new ActionWorldDeleteLink(i), d);

	for (int i=w->lights.num-1; i>=0; i--)
		if (w->lights[i].is_selected)
			addSubAction(new ActionWorldDeleteLight(i), d);

	return NULL;
}

