/*
 * ActionWorldDeleteLight.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#include "ActionWorldDeleteLight.h"

#include "../../../data/world/DataWorld.h"
#include "../../../data/world/WorldLight.h"

ActionWorldDeleteLight::ActionWorldDeleteLight(int i) {
	index = i;
}

void* ActionWorldDeleteLight::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	light = w->lights[index];
	w->lights.erase(index);
	return nullptr;
}

void ActionWorldDeleteLight::undo(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	w->lights.insert(light, index);
}
