/*
 * ActionCameraMoveSelection.cpp
 *
 *  Created on: 02.10.2012
 *      Author: michi
 */

#include "ActionCameraMoveSelection.h"
#include "../../../Data/World/DataCamera.h"

ActionCameraMoveSelection::ActionCameraMoveSelection(DataCamera *d, const vector &_param, const vector &_pos0) :
	ActionMultiView()
{
	// list of selected objects and save old pos
	foreachi(WorldCamPoint &c, d->Point, i)
		if (c.is_selected){
			index.add(i);
			old_data.add(c.pos);
		}else if (d->Vel[i].is_selected){
			index_vel.add(i);
			old_vel.add(c.Vel);
		}
}

void *ActionCameraMoveSelection::execute(Data *d)
{
	/*DataCamera *w = dynamic_cast<DataCamera*>(d);
	foreachi(int i, index, ii)
		w->Point[i].pos = old_data[ii] + param;
	foreachi(int i, index_vel, ii)
		w->Point[i].Vel = old_vel[ii] + param;*/
	return NULL;
}



void ActionCameraMoveSelection::undo(Data *d)
{
	DataCamera *w = dynamic_cast<DataCamera*>(d);
	foreachi(int i, index, ii)
		w->Point[i].pos = old_data[ii];
	foreachi(int i, index_vel, ii)
		w->Point[i].Vel = old_vel[ii];
}

