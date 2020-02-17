/*
 * ActionWorldRotateObjects.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "ActionWorldRotateObjects.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Edward.h"

ActionWorldRotateObjects::ActionWorldRotateObjects(DataWorld *d) :
	ActionMultiView()
{
	// list of selected objects and save old pos
	foreachi(auto &o, d->Objects, i)
		if (o.is_selected) {
			index.add(i);
			old_data.add(o.pos);
			old_ang.add(o.ang);
			type.add(MVD_WORLD_OBJECT);
		}
	foreachi(auto &o, d->cameras, i)
		if (o.is_selected) {
			index.add(i);
			old_data.add(o.pos);
			old_ang.add(o.ang);
			type.add(MVD_WORLD_CAMERA);
		}
	foreachi(auto &o, d->lights, i)
		if (o.is_selected) {
			index.add(i);
			old_data.add(o.pos);
			old_ang.add(o.ang);
			type.add(MVD_WORLD_LIGHT);
		}
}

void ActionWorldRotateObjects::undo(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	foreachi(int i, index, ii) {
		if (type[ii] == MVD_WORLD_OBJECT) {
			w->Objects[i].pos = old_data[ii];
			w->Objects[i].ang = old_ang[ii];
		} else if (type[ii] == MVD_WORLD_CAMERA) {
			w->cameras[i].pos = old_data[ii];
			w->cameras[i].ang = old_ang[ii];
		} else if (type[ii] == MVD_WORLD_LIGHT) {
			w->lights[i].pos = old_data[ii];
			w->lights[i].ang = old_ang[ii];
		}
	}
}



void *ActionWorldRotateObjects::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	auto q = quaternion::rotation_m(mat);
	foreachi(int i, index, ii) {
		if (type[ii] == MVD_WORLD_OBJECT) {
			w->Objects[i].pos = mat * old_data[ii];
			w->Objects[i].ang = VecAngAdd(old_ang[ii], q.get_angles());
		} else if (type[ii] == MVD_WORLD_CAMERA) {
			w->cameras[i].pos = mat * old_data[ii];
			w->cameras[i].ang = VecAngAdd(old_ang[ii], q.get_angles());
		} else if (type[ii] == MVD_WORLD_LIGHT) {
			w->lights[i].pos = mat * old_data[ii];
			w->lights[i].ang = VecAngAdd(old_ang[ii], q.get_angles());
		}
	}
	return NULL;
}


