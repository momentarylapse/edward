//
// Created by Michael Ankele on 2025-02-03.
//

#include "ActionWorldEditComponent.h"
#include "../../data/DataWorld.h"
#include <y/Entity.h>

#include "y/EntityManager.h"


ActionWorldEditBaseEntity::ActionWorldEditBaseEntity(int _index, const vec3& _pos, const quaternion& _ang) {
	pos = _pos;
	ang = _ang;
	index = _index;
}

void* ActionWorldEditBaseEntity::execute(Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	auto e = w->entity(index);
	std::swap(e->pos, pos);
	std::swap(e->ang, ang);
	return nullptr;
}

void ActionWorldEditBaseEntity::undo(Data* d) {
	execute(d);
}


ActionWorldEditComponent::ActionWorldEditComponent(int _index, int _cindex, const ScriptInstanceData& c) {
	index = _index;
	cindex = _cindex;
	component = c;
}

void* ActionWorldEditComponent::execute(Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	std::swap(w->entities[index].components[cindex], component);
	return nullptr;
}

void ActionWorldEditComponent::undo(Data* d) {
	execute(d);
}

ActionWorldAddComponent::ActionWorldAddComponent(int _index, const kaba::Class* _type, const base::map<string, Any>& _variables) {
	index = _index;
	type = _type;
	variables = _variables;
}

void *ActionWorldAddComponent::execute(Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	auto e = w->entity(index);
	component = w->entity_manager->_add_component_generic_(e, type, variables);
	return component;
}

void ActionWorldAddComponent::undo(Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	auto e = w->entity(index);
	w->entity_manager->delete_component(e, component);
}


ActionWorldRemoveComponent::ActionWorldRemoveComponent(int _index, int _cindex) {
	index = _index;
	cindex = _cindex;
}

void *ActionWorldRemoveComponent::execute(Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	component = w->entities[index].components[cindex];
	w->entities[index].components.erase(cindex);
	w->out_component_removed();
	return nullptr;
}

void ActionWorldRemoveComponent::undo(Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	w->entities[index].components.insert(component, cindex);
	w->out_component_added();
}

