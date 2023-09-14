/*
 * ActionWorldAddObject.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "ActionWorldAddObject.h"
#include "../../../data/world/DataWorld.h"
#include "../../../data/world/WorldObject.h"
#include <assert.h>

ActionWorldAddObject::ActionWorldAddObject(const WorldObject &o) {
	object = o;
}

void ActionWorldAddObject::undo(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	assert(w->objects.num > 0);

	w->objects.pop();
}



void *ActionWorldAddObject::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	w->objects.add(object);

	return &w->objects.back();
}


