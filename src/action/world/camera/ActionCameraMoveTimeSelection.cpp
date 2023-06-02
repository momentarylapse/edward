/*
 * ActionCameraMoveTimeSelection.cpp
 *
 *  Created on: 02.10.2012
 *      Author: michi
 */

#include "ActionCameraMoveTimeSelection.h"
#include "../../../data/world/DataCamera.h"

ActionCameraMoveTimeSelection::ActionCameraMoveTimeSelection(DataCamera *d, float _param, float _pos0)
{
	pos0 = _pos0;
	param = _param;
	// list of selected objects and save old pos
	foreachi(WorldCamPoint &c, d->Point, i){
		old_data.add(c.Duration);
		if (c.is_selected)
			index.add(i);
	}
}

void *ActionCameraMoveTimeSelection::execute(Data *d)
{
	DataCamera *w = dynamic_cast<DataCamera*>(d);
	bool last_selected = false;
	foreachi(WorldCamPoint &c, w->Point, i)
		if (c.Type == CPKCamFlight){
			bool selected = index.contains(i);
			if ((selected) && (!last_selected))
				c.Duration += param - pos0;
			else if ((!selected) && (last_selected))
				c.Duration -= param - pos0;
			last_selected = selected;
		}
	/*foreachi(int i, index, ii)
		w->Point[i].pos = old_data[ii] + param;
	foreachi(int i, index_vel, ii)
		w->Point[i].Vel = old_vel[ii] + param;*/
	return NULL;
}



void ActionCameraMoveTimeSelection::undo(Data *d)
{
	DataCamera *w = dynamic_cast<DataCamera*>(d);
	foreachi(WorldCamPoint &c, w->Point, i)
		c.Duration = old_data[i];
}

