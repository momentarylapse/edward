//
// Created by Michael Ankele on 2025-02-03.
//

#include "ActionWorldEditComponent.h"
#include "../../data/DataWorld.h"


ActionWorldEditLight::ActionWorldEditLight(int _index, const WorldLight &l) {
	light = l;
	index = _index;
}

void* ActionWorldEditLight::execute(Data *d) {
	auto w = dynamic_cast<DataWorld*>(d);
	std::swap(w->entities[index].light, light);
	return nullptr;
}

void ActionWorldEditLight::undo(Data *d) {
	auto w = dynamic_cast<DataWorld*>(d);
	std::swap(w->entities[index].light, light);
}


ActionWorldEditCamera::ActionWorldEditCamera(int _index, const WorldCamera& c) {
	camera = c;
	index = _index;
}

void* ActionWorldEditCamera::execute(Data *d) {
	auto w = dynamic_cast<DataWorld*>(d);
	std::swap(w->entities[index].camera, camera);
	return nullptr;
}

void ActionWorldEditCamera::undo(Data *d) {
	auto w = dynamic_cast<DataWorld*>(d);
	std::swap(w->entities[index].camera, camera);
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

ActionWorldAddComponent::ActionWorldAddComponent(int _index, const ScriptInstanceData& c) {
	index = _index;
	component = c;
}

void *ActionWorldAddComponent::execute(Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	w->entities[index].components.add(component);
	return &w->entities[index].components;
}

void ActionWorldAddComponent::undo(Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	w->entities[index].components.pop();
}


ActionWorldRemoveComponent::ActionWorldRemoveComponent(int _index, int _cindex) {
	index = _index;
	cindex = _cindex;
}

void *ActionWorldRemoveComponent::execute(Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	component = w->entities[index].components[cindex];
	w->entities[index].components.erase(cindex);
	return nullptr;
}

void ActionWorldRemoveComponent::undo(Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);
	w->entities[index].components.insert(component, cindex);
}

