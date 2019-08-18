/*
 * ActionWorldRotateObjects.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "ActionWorldRotateObjects.h"
#include "../../../Data/World/DataWorld.h"

ActionWorldRotateObjects::ActionWorldRotateObjects(DataWorld *d) :
	ActionMultiView()
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
	foreachi(int i, index, ii){
		w->Objects[i].pos = old_data[ii];
		w->Objects[i].Ang = old_ang[ii];
	}
}



void *ActionWorldRotateObjects::execute(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	quaternion q;
	q = quaternion::rotation_m( mat);
	foreachi(int i, index, ii){
		w->Objects[i].pos = mat * old_data[ii];
		w->Objects[i].Ang = VecAngAdd(old_ang[ii], q.get_angles());
	}
	return NULL;
}


