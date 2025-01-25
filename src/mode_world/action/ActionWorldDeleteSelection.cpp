/*
 * ActionWorldDeleteSelection.cpp
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#include "ActionWorldDeleteSelection.h"
#include <Session.h>
#include <lib/base/iter.h>
#include <lib/base/sort.h>

ActionWorldDeleteSelection::ActionWorldDeleteSelection(DataWorld* w, const Data::Selection& selection) {
	if (selection.contains(MultiViewType::WORLD_OBJECT))
		for (const auto& [i, o]: enumerate(w->objects))
			if (selection[MultiViewType::WORLD_OBJECT].contains(i)) {
				objects.add(o);
				object_indices.add(i);
			}
	if (selection.contains(MultiViewType::WORLD_TERRAIN))
		for (const auto& [i, o]: enumerate(w->terrains))
			if (selection[MultiViewType::WORLD_TERRAIN].contains(i)) {
				terrains.add(o);
				terrain_indices.add(i);
			}
	if (selection.contains(MultiViewType::WORLD_CAMERA))
		for (const auto& [i, o]: enumerate(w->cameras))
			if (selection[MultiViewType::WORLD_CAMERA].contains(i)) {
				cameras.add(o);
				camera_indices.add(i);
			}
	if (selection.contains(MultiViewType::WORLD_LIGHT))
		for (const auto& [i, o]: enumerate(w->lights))
			if (selection[MultiViewType::WORLD_LIGHT].contains(i)) {
				lights.add(o);
				light_indices.add(i);
			}
	if (selection.contains(MultiViewType::WORLD_LINK))
		for (const auto& [i, o]: enumerate(w->links))
			if (selection[MultiViewType::WORLD_LINK].contains(i)) {
				links.add(o);
				link_indices.add(i);
			}
}

void *ActionWorldDeleteSelection::execute(Data *d) {
	auto w = dynamic_cast<DataWorld*>(d);
	for (int i: base::reverse(object_indices))
		w->objects.erase(i);
	for (int i: base::reverse(terrain_indices))
		w->terrains.erase(i);
	for (int i: base::reverse(camera_indices))
		w->cameras.erase(i);
	for (int i: base::reverse(light_indices))
		w->lights.erase(i);
	for (int i: base::reverse(link_indices))
		w->links.erase(i);
	return nullptr;
}

void ActionWorldDeleteSelection::undo(Data *d) {
	auto w = dynamic_cast<DataWorld*>(d);
	for (const auto& [ii, i]: enumerate(object_indices))
		w->objects.insert(objects[ii], i);
	for (const auto& [ii, i]: enumerate(terrain_indices))
		w->terrains.insert(terrains[ii], i);
	for (const auto& [ii, i]: enumerate(camera_indices))
		w->cameras.insert(cameras[ii], i);
	for (const auto& [ii, i]: enumerate(light_indices))
		w->lights.insert(lights[ii], i);
	for (const auto& [ii, i]: enumerate(link_indices))
		w->links.insert(links[ii], i);
}

