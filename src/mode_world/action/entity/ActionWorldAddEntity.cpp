//
// Created by Michael Ankele on 2025-01-31.
//

#include "ActionWorldAddEntity.h"
#include "../../data/DataWorld.h"
#include <ecs/EntityManager.h>

ActionWorldAddEntity::ActionWorldAddEntity(const vec3& _pos, const quaternion& _ang) {
	pos = _pos;
	ang = _ang;
}

void* ActionWorldAddEntity::execute(Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	entity = w->entity_manager->create_entity(pos, ang);
	w->entity_manager->add_component<EdwardTag>(entity);
	return entity;
}

void ActionWorldAddEntity::undo(Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	w->entity_manager->delete_entity(entity);
}
