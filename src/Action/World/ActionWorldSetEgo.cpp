/*
 * ActionWorldSetEgo.cpp
 *
 *  Created on: 15.06.2012
 *      Author: michi
 */

#include "ActionWorldSetEgo.h"
#include "../../Data/World/DataWorld.h"

ActionWorldSetEgo::ActionWorldSetEgo(int _index)
{
	index = _index;
}

ActionWorldSetEgo::~ActionWorldSetEgo()
{
}

void ActionWorldSetEgo::undo(Data *d)
{
	execute(d);
}



void *ActionWorldSetEgo::execute(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);

	// swap
	int old_index = w->EgoIndex;
	w->EgoIndex = index;
	index = old_index;

	return NULL;
}


