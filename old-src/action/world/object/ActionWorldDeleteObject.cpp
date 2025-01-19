/*
 * ActionWorldDeleteObject.cpp
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#include "ActionWorldDeleteObject.h"
#include "../../../data/world/DataWorld.h"
#include <assert.h>

ActionWorldDeleteObject::ActionWorldDeleteObject(int _index) {
	index = _index;
}

void* ActionWorldDeleteObject::execute(Data* d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	assert(index >= 0);
	assert(index < w->objects.num);

	object = w->objects[index];
	w->objects.erase(index);

	return NULL;
}

void ActionWorldDeleteObject::undo(Data* d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);

	object.is_selected = false;
	w->objects.insert(object, index);
}
