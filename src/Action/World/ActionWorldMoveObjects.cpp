/*
 * ActionWorldMoveObjects.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "ActionWorldMoveObjects.h"
#include "../../Data/World/DataWorld.h"

ActionWorldMoveObjects::ActionWorldMoveObjects(Data *d, const vector &_pos0) :
	ActionMultiView(d, _pos0)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);

	// list of selected objects and save old pos
	foreachi(w->Object, o, i)
		if (o.is_selected){
			index.add(i);
			old_data.add(o.pos);
		}
}

ActionWorldMoveObjects::~ActionWorldMoveObjects()
{
}

void *ActionWorldMoveObjects::execute(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	foreachi(index, i, ii)
		w->Object[i].pos = old_data[ii] + param;
	return NULL;
}



void ActionWorldMoveObjects::abort(Data *d)
{
	undo(d);
}



void ActionWorldMoveObjects::undo(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	foreachi(index, i, ii)
		w->Object[i].pos = old_data[ii];
}


