/*
 * ActionWorldAddObject.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "ActionWorldAddObject.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../x/object.h"
#include "../../../x/model_manager.h"
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
	assert(w->Objects.num > 0);

	w->Objects.pop();
}



void *ActionWorldAddObject::execute(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);

	WorldObject o;
	o.pos = pos;
	o.ang = ang;
	o.is_selected = true;
	o.is_special = false;
	o.filename = filename;
	o.view_stage = 0;//mode_world->ViewStage;
	o.object = (Object*)LoadModel(filename);
	w->Objects.add(o);

	return &w->Objects.back();
}


