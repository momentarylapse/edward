/*
 * ActionWorldAddObject.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "ActionWorldAddObject.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../x/object.h"
#include <assert.h>

#include "../../../x/ModelManager.h"

ActionWorldAddObject::ActionWorldAddObject(const string &_filename, const vector &_pos, const vector &_ang) {
	filename = _filename;
	pos = _pos;
	ang = _ang;
}

void ActionWorldAddObject::undo(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	assert(w->objects.num > 0);

	w->objects.pop();
}



void *ActionWorldAddObject::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);

	WorldObject o;
	o.pos = pos;
	o.ang = ang;
	o.is_selected = true;
	o.is_special = false;
	o.filename = filename;
	o.view_stage = 0;//mode_world->ViewStage;
	o.object = (Object*)ModelManager::load(filename);
	w->objects.add(o);

	return &w->objects.back();
}


