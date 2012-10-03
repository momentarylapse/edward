/*
 * ActionCameraMoveSelection.cpp
 *
 *  Created on: 02.10.2012
 *      Author: michi
 */

#include "ActionCameraMoveSelection.h"
#include "../../../Data/World/DataCamera.h"

ActionCameraMoveSelection::ActionCameraMoveSelection(DataCamera *d, const vector &_param, const vector &_pos0) :
	ActionMultiView(_param, _pos0)
{
	// list of selected objects and save old pos
	foreachi(WorldCamPoint &c, d->Point, i)
		if (c.is_selected){
			index.add(i);
			old_data.add(c.pos);
		}
}

void *ActionCameraMoveSelection::execute(Data *d)
{
	DataCamera *w = dynamic_cast<DataCamera*>(d);
	foreachi(int i, index, ii)
		w->Point[i].pos = old_data[ii] + param;
	return NULL;
}



void ActionCameraMoveSelection::undo(Data *d)
{
	DataCamera *w = dynamic_cast<DataCamera*>(d);
	foreachi(int i, index, ii)
		w->Point[i].pos = old_data[ii];
}

