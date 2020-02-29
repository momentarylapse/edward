/*
 * ActionWorldDeleteObject.cpp
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#include "ActionWorldDeleteObject.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../x/object.h"
#include <assert.h>

#include "../../../x/ModelManager.h"

ActionWorldDeleteObject::ActionWorldDeleteObject(int _index)
{
	index = _index;
}

ActionWorldDeleteObject::~ActionWorldDeleteObject()
{
}

void* ActionWorldDeleteObject::execute(Data* d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	assert(index >= 0);
	assert(index < w->Objects.num);

	object = w->Objects[index];
	w->Objects.erase(index);

	return NULL;
}

void ActionWorldDeleteObject::undo(Data* d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);

	object.is_selected = false;
	w->Objects.insert(object, index);
}
