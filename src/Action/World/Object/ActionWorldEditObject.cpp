/*
 * ActionWorldEditObject.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "ActionWorldEditObject.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Data/World/WorldObject.h"
#include <assert.h>
#include <algorithm>

ActionWorldEditObject::ActionWorldEditObject(int _index, const WorldObject &_data) {
	data = _data;
	index = _index;
}

void ActionWorldEditObject::undo(Data *d) {
	execute(d);
}



void *ActionWorldEditObject::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	assert(index >= 0);
	assert(index < w->objects.num);

	std::swap(data, w->objects[index]);

	return NULL;
}


