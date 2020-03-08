/*
 * ActionWorldMoveObjects.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "ActionWorldMoveSelection.h"
#include "../../Data/World/DataWorld.h"
#include "../../Data/World/WorldCamera.h"
#include "../../Data/World/WorldLight.h"
#include "../../Data/World/WorldLink.h"
#include "../../Data/World/WorldObject.h"
#include "../../Data/World/WorldTerrain.h"
#include "../../x/terrain.h"
#include "../../Edward.h"

ActionWorldMoveSelection::ActionWorldMoveSelection(DataWorld *d) :
	ActionMultiView()
{
	// list of selected objects and save old pos
	foreachi(auto &o, d->objects, i)
		if (o.is_selected) {
			index.add(i);
			old_data.add(o.pos);
			type.add(MVD_WORLD_OBJECT);
		}
	foreachi(auto &t, d->terrains, i)
		if (t.is_selected) {
			index.add(i);
			old_data.add(t.pos);
			type.add(MVD_WORLD_TERRAIN);
		}
	foreachi(auto &c, d->cameras, i)
		if (c.is_selected) {
			index.add(i);
			old_data.add(c.pos);
			type.add(MVD_WORLD_CAMERA);
		}
	foreachi(auto &l, d->lights, i)
		if (l.is_selected) {
			index.add(i);
			old_data.add(l.pos);
			type.add(MVD_WORLD_LIGHT);
		}
	foreachi(auto &l, d->links, i)
		if (l.is_selected) {
			index.add(i);
			old_data.add(l.pos);
			type.add(MVD_WORLD_LINK);
		}
}

void *ActionWorldMoveSelection::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	foreachi(int i, index, ii) {
		if (type[ii] == MVD_WORLD_OBJECT) {
			w->objects[i].pos = mat * old_data[ii];
		} else if (type[ii] == MVD_WORLD_TERRAIN) {
			w->terrains[i].pos = mat * old_data[ii];
			w->terrains[i].terrain->pos = w->terrains[i].pos;
			w->terrains[i].terrain->update(-1, -1, -1, -1, TerrainUpdateVertices);
		} else if (type[ii] == MVD_WORLD_LIGHT) {
			w->lights[i].pos = mat * old_data[ii];
		} else if (type[ii] == MVD_WORLD_CAMERA) {
			w->cameras[i].pos = mat * old_data[ii];
		} else if (type[ii] == MVD_WORLD_LINK) {
			w->links[i].pos = mat * old_data[ii];
		}
	}
	return NULL;
}



void ActionWorldMoveSelection::undo(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	foreachi(int i, index, ii) {
		if (type[ii] == MVD_WORLD_OBJECT) {
			w->objects[i].pos = old_data[ii];
		} else if (type[ii] == MVD_WORLD_TERRAIN) {
			w->terrains[i].pos = old_data[ii];
			w->terrains[i].terrain->pos = w->terrains[i].pos;
			w->terrains[i].terrain->update(-1, -1, -1, -1, TerrainUpdateVertices);
		} else if (type[ii] == MVD_WORLD_LIGHT) {
			w->lights[i].pos = old_data[ii];
		} else if (type[ii] == MVD_WORLD_CAMERA) {
			w->cameras[i].pos = old_data[ii];
		} else if (type[ii] == MVD_WORLD_LINK) {
			w->links[i].pos = old_data[ii];
		}
	}
}


