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
