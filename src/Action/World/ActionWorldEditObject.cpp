/*
 * ActionWorldEditObject.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "ActionWorldEditObject.h"
#include <assert.h>

ActionWorldEditObject::ActionWorldEditObject(int _index, const ModeWorldObject &_data)
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
	assert(index < w->Object.num);

	// swap
	ModeWorldObject old_data = w->Object[index];
	w->Object[index] = data;
	data = old_data;

	return NULL;
}


