/*
 * ActionWorldPaste.cpp
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#include "ActionWorldPaste.h"
#include "../data/DataWorld.h"
#include <y/EntityManager.h>


ActionWorldPaste::ActionWorldPaste(const LevelData& _temp) {
	temp = _temp;
}

void* ActionWorldPaste::execute(Data *d) {
	auto *w = dynamic_cast<DataWorld*>(d);

	for (const auto& o: temp.entities) {
		auto e = w->_create_entity(o.pos, o.ang);
		w->_entity_apply_components(e, o.components);
	}

	return nullptr;
}

void ActionWorldPaste::undo(Data *d) {
	auto *w = dynamic_cast<DataWorld*>(d);
	for (int i=0; i<temp.entities.num; i++)
		w->entity_manager->delete_entity(w->entity_manager->entities.back());

}


