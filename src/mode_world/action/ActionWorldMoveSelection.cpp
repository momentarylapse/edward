/*
 * ActionWorldMoveObjects.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "ActionWorldMoveSelection.h"

#include <lib/math/quaternion.h>

#include "../data/DataWorld.h"
#include "../data/WorldCamera.h"
#include "../data/WorldLight.h"
#include "../data/WorldLink.h"
#include "../data/WorldObject.h"
#include "../data/WorldTerrain.h"
#include <y/world/Terrain.h>
#include "../../Session.h"

ActionWorldMoveSelection::ActionWorldMoveSelection(DataWorld *d, const Data::Selection& selection) :
	ActionMultiView()
{
	// list of selected objects and save old pos
	if (selection.contains(MultiViewType::WORLD_OBJECT))
		foreachi(auto &o, d->objects, i)
			if (selection[MultiViewType::WORLD_OBJECT].contains(i)) {
				index.add(i);
				old_data.add(o.pos);
				old_ang.add(quaternion::rotation(o.ang));
				type.add(MultiViewType::WORLD_OBJECT);
			}
	if (selection.contains(MultiViewType::WORLD_TERRAIN))
		foreachi(auto &t, d->terrains, i)
			if (selection[MultiViewType::WORLD_TERRAIN].contains(i)) {
				index.add(i);
				old_data.add(t.pos);
				old_ang.add(quaternion::ID);
				type.add(MultiViewType::WORLD_TERRAIN);
			}
	if (selection.contains(MultiViewType::WORLD_CAMERA))
		foreachi(auto &c, d->cameras, i)
			if (selection[MultiViewType::WORLD_CAMERA].contains(i)) {
				index.add(i);
				old_data.add(c.pos);
				old_ang.add(quaternion::rotation(c.ang));
				type.add(MultiViewType::WORLD_CAMERA);
			}
	if (selection.contains(MultiViewType::WORLD_LIGHT))
		foreachi(auto &l, d->lights, i)
			if (selection[MultiViewType::WORLD_LIGHT].contains(i)) {
				index.add(i);
				old_data.add(l.pos);
				old_ang.add(quaternion::rotation(l.ang));
				type.add(MultiViewType::WORLD_LIGHT);
			}
	if (selection.contains(MultiViewType::WORLD_LINK))
		foreachi(auto &l, d->links, i)
			if (selection[MultiViewType::WORLD_LINK].contains(i)) {
				index.add(i);
				old_data.add(l.pos);
				old_ang.add(quaternion::ID);
				type.add(MultiViewType::WORLD_LINK);
			}
}

void *ActionWorldMoveSelection::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	auto dq = quaternion::rotation(mat);
	foreachi(int i, index, ii) {
		if (type[ii] == MultiViewType::WORLD_OBJECT) {
			w->objects[i].pos = mat * old_data[ii];
			w->objects[i].ang = (dq * old_ang[ii]).get_angles();
		} else if (type[ii] == MultiViewType::WORLD_TERRAIN) {
			w->terrains[i].pos = mat * old_data[ii];
			//w->terrains[i].terrain->update(-1, -1, -1, -1, TerrainUpdateVertices);
		} else if (type[ii] == MultiViewType::WORLD_LIGHT) {
			w->lights[i].pos = mat * old_data[ii];
			w->lights[i].ang = (dq * old_ang[ii]).get_angles();
		} else if (type[ii] == MultiViewType::WORLD_CAMERA) {
			w->cameras[i].pos = mat * old_data[ii];
			w->cameras[i].ang = (dq * old_ang[ii]).get_angles();
		} else if (type[ii] == MultiViewType::WORLD_LINK) {
			w->links[i].pos = mat * old_data[ii];
		}
	}
	return NULL;
}



void ActionWorldMoveSelection::undo(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	foreachi(int i, index, ii) {
		if (type[ii] == MultiViewType::WORLD_OBJECT) {
			w->objects[i].pos = old_data[ii];
			w->objects[i].ang = old_ang[ii].get_angles();
		} else if (type[ii] == MultiViewType::WORLD_TERRAIN) {
			w->terrains[i].pos = old_data[ii];
			//w->terrains[i].terrain->update(-1, -1, -1, -1, TerrainUpdateVertices);
		} else if (type[ii] == MultiViewType::WORLD_LIGHT) {
			w->lights[i].pos = old_data[ii];
			w->lights[i].ang = old_ang[ii].get_angles();
		} else if (type[ii] == MultiViewType::WORLD_CAMERA) {
			w->cameras[i].pos = old_data[ii];
			w->cameras[i].ang = old_ang[ii].get_angles();
		} else if (type[ii] == MultiViewType::WORLD_LINK) {
			w->links[i].pos = old_data[ii];
		}
	}
}


