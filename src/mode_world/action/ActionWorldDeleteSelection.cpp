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
	w->copy(temp, selection);
	if (selection.contains(MultiViewType::WORLD_ENTITY))
		for (auto&& [i, e]: enumerate(w->entity_manager->entities))
			if (selection[MultiViewType::WORLD_ENTITY].contains(i)) {
				entity_indices.add(i);
			}
	if (selection.contains(MultiViewType::WORLD_LINK))
		for (const auto& [i, o]: enumerate(w->links))
			if (selection[MultiViewType::WORLD_LINK].contains(i)) {
		//		link_indices.add(i);
			}
}

void *ActionWorldDeleteSelection::execute(Data *d) {
	auto w = dynamic_cast<DataWorld*>(d);

	for (int i: base::reverse(entity_indices))
		w->entity_manager->delete_entity(w->entity(i));

	for (int i: base::reverse(link_indices))
		w->links.erase(i);
	return nullptr;
}

void ActionWorldDeleteSelection::undo(Data *d) {
	auto w = dynamic_cast<DataWorld*>(d);

	// TODO keep indices...
	entity_indices.clear();
	for (const auto& [i, ee]: enumerate(temp.entities)) {
		entity_indices.add(w->entity_manager->entities.num);
		auto e = w->entity_manager->create_entity(ee.pos, ee.ang);
		w->entity_manager->add_component<EdwardTag>(e);
		w->_entity_apply_components(e, ee.components);
	}


//	for (const auto& [ii, i]: enumerate(link_indices))
//		w->links.insert(temp.links[ii], i);
}

