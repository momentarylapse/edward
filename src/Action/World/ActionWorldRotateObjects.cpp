/*
 * ActionWorldRotateObjects.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "ActionWorldRotateObjects.h"
#include "../../Data/World/DataWorld.h"

ActionWorldRotateObjects::ActionWorldRotateObjects(Data *d, const vector &_pos0) :
	ActionMultiView(d, _pos0)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);

	// list of selected objects and save old pos
	foreachi(w->Object, o, i)
		if (o.is_selected){
			index.add(i);
			old_data.add(o.pos);
			old_ang.add(o.Ang);
		}
}

ActionWorldRotateObjects::~ActionWorldRotateObjects()
{
}

void ActionWorldRotateObjects::undo(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	matrix rot;
	MatrixRotation(rot, param);
	foreachi(index, i, ii){
		w->Object[i].pos = old_data[ii];
		w->Object[i].Ang = old_ang[ii];
	}
}



void ActionWorldRotateObjects::abort(Data *d)
{
	undo(d);
}



void *ActionWorldRotateObjects::execute(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	matrix rot;
	MatrixRotation(rot, param);
	foreachi(index, i, ii){
		w->Object[i].pos = pos0 + rot * (old_data[ii] - pos0);
		w->Object[i].Ang = VecAngAdd(old_ang[ii], param);
	}
	return NULL;
}


