/*
 * ActionWorldEditObject.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "ActionWorldEditObject.h"
#include <assert.h>

ActionWorldEditObject::ActionWorldEditObject(int _index, const WorldObject &_data)
{
	data = _data;
	index = _index;
}

ActionWorldEditObject::~ActionWorldEditObject()
{
}

void ActionWorldEditObject::undo(Data *d)
{
	execute(d);
}



void *ActionWorldEditObject::execute(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	assert(index >= 0);
	assert(index < w->Objects.num);

	// swap
	WorldObject old_data = w->Objects[index];
	w->Objects[index] = data;
	data = old_data;

	return NULL;
}


