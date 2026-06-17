/*
 * ActionWorldMoveObjects.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "ActionWorldMoveSelection.h"
#include <lib/base/iter.h>
#include <lib/math/quaternion.h>
#include "../data/DataWorld.h"
#include <ecs/EntityManager.h>
#include <ecs/Entity.h>

ActionWorldMoveSelection::ActionWorldMoveSelection(DataWorld *d, const Selection& selection) {
	// list of selected objects and save old pos
	if (selection.contains(MultiViewType::WORLD_ENTITY))
		for (const auto& [i, o]: enumerate(d->entity_manager->entities))
			if (selection[MultiViewType::WORLD_ENTITY].contains(i)) {
				index.add(i);
				old_data.add(o->pos);
				old_ang.add(o->ang);
				type.add(MultiViewType::WORLD_ENTITY);
			}
}

void *ActionWorldMoveSelection::execute(history::Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	auto dq = quaternion::rotation(mat);
	for (const auto& [ii, i]: enumerate(index)) {
		if (type[ii] == MultiViewType::WORLD_ENTITY) {
			w->entity(i)->pos = mat * old_data[ii];
			w->entity(i)->ang = dq * old_ang[ii];
		}
	}
	return nullptr;
}

void ActionWorldMoveSelection::undo(history::Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	for (const auto& [ii, i]: enumerate(index)) {
		if (type[ii] == MultiViewType::WORLD_ENTITY) {
			w->entity(i)->pos = old_data[ii];
			w->entity(i)->ang = old_ang[ii];
		}
	}
}


