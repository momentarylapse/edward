/*
 * ActionWorldRotateObjects.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "ActionWorldRotateObjects.h"
#include "../../../Data/World/DataWorld.h"

ActionWorldRotateObjects::ActionWorldRotateObjects(DataWorld *d, const vector &_param, const vector &_pos0) :
	ActionMultiView(_param, _pos0)
{
	// list of selected objects and save old pos
	foreachi(WorldObject &o, d->Objects, i)
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
	foreachi(int i, index, ii){
		w->Objects[i].pos = old_data[ii];
		w->Objects[i].Ang = old_ang[ii];
	}
}



void *ActionWorldRotateObjects::execute(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	matrix rot;
	MatrixRotation(rot, param);
	foreachi(int i, index, ii){
		w->Objects[i].pos = pos0 + rot * (old_data[ii] - pos0);
		w->Objects[i].Ang = VecAngAdd(old_ang[ii], param);
	}
	return NULL;
}


