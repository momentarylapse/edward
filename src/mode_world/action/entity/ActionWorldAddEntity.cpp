//
// Created by Michael Ankele on 2025-01-31.
//

#include "ActionWorldAddEntity.h"
#include "../../data/DataWorld.h"

ActionWorldAddEntity::ActionWorldAddEntity(const WorldEntity& e) {
	entity = e;
}

void* ActionWorldAddEntity::execute(Data* d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	w->entities.add(entity);
	return &w->entities.back();
}

void ActionWorldAddEntity::undo(Data* d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	w->entities.pop();
}
