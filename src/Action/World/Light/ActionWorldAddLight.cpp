/*
 * ActionWorldAddLight.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#include "ActionWorldAddLight.h"

#include "../../../Data/World/DataWorld.h"
#include "../../../Data/World/WorldLight.h"

ActionWorldAddLight::ActionWorldAddLight(WorldLight &l) {
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
