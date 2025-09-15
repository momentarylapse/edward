/*
 * ActionWorldDeleteSelection.cpp
 *
 *  Created on: 06.09.2013
 *      Author: michi
 */

#include "ActionWorldDeleteSelection.h"
#include <lib/base/iter.h>
#include <lib/base/sort.h>

#include "y/EntityManager.h"

ActionWorldDeleteSelection::ActionWorldDeleteSelection(DataWorld* w, const Data::Selection& selection) {
	if (selection.contains(MultiViewType::WORLD_ENTITY))
		for (auto t: w->entity_manager->get_component_list<EdwardTag>())
			if (selection[MultiViewType::WORLD_ENTITY].contains(t->entity_index)) {
				//entities.add(o);
				entity_indices.add(t->entity_index);
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
	/*for (int i: base::reverse(entity_indices))
		w->entities.erase(i);*/
	for (int i: base::reverse(link_indices))
		w->links.erase(i);
	return nullptr;
}

void ActionWorldDeleteSelection::undo(Data *d) {
	auto w = dynamic_cast<DataWorld*>(d);
	/*for (const auto& [ii, i]: enumerate(entity_indices))
		w->entities.insert(entities[ii], i);*/
	for (const auto& [ii, i]: enumerate(link_indices))
		w->links.insert(links[ii], i);
}

