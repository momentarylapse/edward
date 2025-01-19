/*
 * ActionWorldAddLight.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#include "ActionWorldAddLight.h"

#include "../../../data/world/DataWorld.h"
#include "../../../data/world/WorldLight.h"

ActionWorldAddLight::ActionWorldAddLight(const WorldLight &l) {
	light = l;
}

void* ActionWorldAddLight::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	w->lights.add(light);
	return &w->lights.back();
}

void ActionWorldAddLight::undo(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	w->lights.pop();
}
