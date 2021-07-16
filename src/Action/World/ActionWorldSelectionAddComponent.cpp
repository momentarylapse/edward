/*
 * ActionWorldMoveObjects.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "ActionWorldSelectionAddComponent.h"
#include "../../Data/World/DataWorld.h"
#include "../../Data/World/WorldCamera.h"
#include "../../Data/World/WorldLight.h"
#include "../../Data/World/WorldLink.h"
#include "../../Data/World/WorldObject.h"
#include "../../Data/World/WorldTerrain.h"
#include "../../y/Terrain.h"
#include "../../Edward.h"

ActionWorldSelectionAddComponent::ActionWorldSelectionAddComponent(DataWorld *d, const ScriptInstanceData &com) {
	component = com;
	foreachi(auto &o, d->objects, i)
		if (o.is_selected) {
			index.add(i);
			type.add(MVD_WORLD_OBJECT);
		}
	foreachi(auto &t, d->terrains, i)
		if (t.is_selected) {
			index.add(i);
			type.add(MVD_WORLD_TERRAIN);
		}
	foreachi(auto &c, d->cameras, i)
		if (c.is_selected) {
			index.add(i);
			type.add(MVD_WORLD_CAMERA);
		}
	foreachi(auto &l, d->lights, i)
		if (l.is_selected) {
			index.add(i);
			type.add(MVD_WORLD_LIGHT);
		}
	foreachi(auto &l, d->links, i)
		if (l.is_selected) {
			index.add(i);
			type.add(MVD_WORLD_LINK);
		}
}

void *ActionWorldSelectionAddComponent::execute(Data *d) {
	auto w = dynamic_cast<DataWorld*>(d);
	foreachi(int i, index, ii) {
		if (type[ii] == MVD_WORLD_OBJECT) {
			w->objects[i].components.add(component);
		} else if (type[ii] == MVD_WORLD_TERRAIN) {
			w->terrains[i].components.add(component);
		} else if (type[ii] == MVD_WORLD_LIGHT) {
			w->lights[i].components.add(component);
		} else if (type[ii] == MVD_WORLD_CAMERA) {
			w->cameras[i].components.add(component);
		} else if (type[ii] == MVD_WORLD_LINK) {
			w->links[i].components.add(component);
		}
	}
	return nullptr;
}



void ActionWorldSelectionAddComponent::undo(Data *d) {
	auto w = dynamic_cast<DataWorld*>(d);
	foreachi(int i, index, ii) {
		if (type[ii] == MVD_WORLD_OBJECT) {
			w->objects[i].components.pop();
		} else if (type[ii] == MVD_WORLD_TERRAIN) {
			w->terrains[i].components.pop();
		} else if (type[ii] == MVD_WORLD_LIGHT) {
			w->lights[i].components.pop();
		} else if (type[ii] == MVD_WORLD_CAMERA) {
			w->cameras[i].components.pop();
		} else if (type[ii] == MVD_WORLD_LINK) {
			w->links[i].components.pop();
		}
	}
}


