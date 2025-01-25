/*
 * ActionWorldDeleteSelection.cpp
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#include "ActionWorldDeleteSelection.h"

#include <Session.h>
#include <lib/os/msg.h>

#include "object/ActionWorldDeleteObject.h"
/*#include "terrain/ActionWorldDeleteTerrain.h"
#include "link/ActionWorldDeleteLink.h"
#include "light/ActionWorldDeleteLight.h"*/

ActionWorldDeleteSelection::ActionWorldDeleteSelection(const Data::Selection& _selection) {
	selection = _selection;
}

void *ActionWorldDeleteSelection::compose(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);

	if (selection.contains(MultiViewType::WORLD_OBJECT))
		for (int i=w->objects.num-1; i>=0; i--)
			if (selection[MultiViewType::WORLD_OBJECT].contains(i))
				addSubAction(new ActionWorldDeleteObject(i), d);

	/*for (int i=w->terrains.num-1; i>=0; i--)
		if (w->terrains[i].is_selected)
			addSubAction(new ActionWorldDeleteTerrain(i), d);

	for (int i=w->links.num-1; i>=0; i--)
		if (w->links[i].is_selected)
			addSubAction(new ActionWorldDeleteLink(i), d);

	for (int i=w->lights.num-1; i>=0; i--)
		if (w->lights[i].is_selected)
			addSubAction(new ActionWorldDeleteLight(i), d);*/

	return nullptr;
}

