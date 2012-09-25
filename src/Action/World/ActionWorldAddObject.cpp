/*
 * ActionWorldAddObject.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "ActionWorldAddObject.h"
#include "../../Data/World/DataWorld.h"
#include "../../lib/x/x.h"
#include <assert.h>

ActionWorldAddObject::ActionWorldAddObject(const string &_filename, const vector &_pos, const vector &_ang)
{
	filename = _filename;
	pos = _pos;
	ang = _ang;
}

ActionWorldAddObject::~ActionWorldAddObject()
{
}

void ActionWorldAddObject::undo(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	assert(w->Object.num > 0);

	w->Object.pop();
}



void *ActionWorldAddObject::execute(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);

	WorldObject o;
	o.pos = pos;
	o.Ang = ang;
	o.is_selected = true;
	o.is_special = false;
	o.FileName = filename;
	o.view_stage = 0;//mode_world->ViewStage;
	o.object = (CObject*)MetaLoadModel(filename);
	w->Object.add(o);

	return &w->Object.back();
}


