/*
 * ActionWorldAddLight.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#include "ActionWorldAddLight.h"
#include "../../data/DataWorld.h"
#include "../../data/WorldLight.h"

ActionWorldAddLight::ActionWorldAddLight(const WorldLight &l) {
	light = l;
}

void* ActionWorldAddLight::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	//w->lights.add(light);
	//return &w->lights.back();
	return nullptr;
}

void ActionWorldAddLight::undo(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	//w->lights.pop();
}
