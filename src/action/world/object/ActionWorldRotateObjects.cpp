/*
 * ActionWorldRotateObjects.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "ActionWorldRotateObjects.h"
#include "../../../data/world/DataWorld.h"
#include "../../../data/world/WorldCamera.h"
#include "../../../data/world/WorldObject.h"
#include "../../../data/world/WorldLight.h"
#include "../../../data/world/WorldLink.h"
#include "../../../Session.h"
#include "../../../lib/math/quaternion.h"

ActionWorldRotateObjects::ActionWorldRotateObjects(DataWorld *d) :
	ActionMultiView()
{
	// list of selected objects and save old pos
	foreachi(auto &o, d->objects, i)
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
	foreachi(auto &o, d->links, i)
		if (o.is_selected) {
			index.add(i);
			old_data.add(o.pos);
			old_ang.add(o.ang);
			type.add(MVD_WORLD_LINK);
		}
}

void ActionWorldRotateObjects::undo(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	foreachi(int i, index, ii) {
		if (type[ii] == MVD_WORLD_OBJECT) {
			w->objects[i].pos = old_data[ii];
			w->objects[i].ang = old_ang[ii];
		} else if (type[ii] == MVD_WORLD_CAMERA) {
			w->cameras[i].pos = old_data[ii];
			w->cameras[i].ang = old_ang[ii];
		} else if (type[ii] == MVD_WORLD_LIGHT) {
			w->lights[i].pos = old_data[ii];
			w->lights[i].ang = old_ang[ii];
		} else if (type[ii] == MVD_WORLD_LINK) {
			w->links[i].pos = old_data[ii];
			w->links[i].ang = old_ang[ii];
		}
	}
}



void *ActionWorldRotateObjects::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	auto q = quaternion::rotation_m(mat);
	foreachi(int i, index, ii) {
		if (type[ii] == MVD_WORLD_OBJECT) {
			w->objects[i].pos = mat * old_data[ii];
			w->objects[i].ang = (q * quaternion::rotation(old_ang[ii])).get_angles();
		} else if (type[ii] == MVD_WORLD_CAMERA) {
			w->cameras[i].pos = mat * old_data[ii];
			w->cameras[i].ang = (q * quaternion::rotation(old_ang[ii])).get_angles();
		} else if (type[ii] == MVD_WORLD_LIGHT) {
			w->lights[i].pos = mat * old_data[ii];
			w->lights[i].ang = (q * quaternion::rotation(old_ang[ii])).get_angles();
		} else if (type[ii] == MVD_WORLD_LINK) {
			w->links[i].pos = mat * old_data[ii];
			w->links[i].ang = (q * quaternion::rotation(old_ang[ii])).get_angles();
		}
	}
	return nullptr;
}


