/*
 * ActionWorldEditLight.cpp
 *
 *  Created on: 9 Sept 2023
 *      Author: michi
 */

#include "ActionWorldEditLight.h"

#include "../../../data/world/DataWorld.h"
#include "../../../data/world/WorldLight.h"

ActionWorldEditLight::ActionWorldEditLight(int _index, const WorldLight &l) {
	index = _index;
	light = l;
}

void* ActionWorldEditLight::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	std::swap(w->lights[index], light);
	return &w->lights[index];
}

void ActionWorldEditLight::undo(Data *d) {
	execute(d);
}

