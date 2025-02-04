/*
 * ActionWorldAddLight.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#include "ActionWorldAddLight.h"

#include <view/MultiView.h>

#include "../../data/DataWorld.h"
#include "../../data/WorldLight.h"

ActionWorldAddLight::ActionWorldAddLight(const WorldLight &l) {
	light = l;
}

void* ActionWorldAddLight::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	WorldEntity e;
	e.basic_type = MultiViewType::WORLD_LIGHT;
	e.light = light;
	w->entities.add(e);
	return &w->entities.back();
}

void ActionWorldAddLight::undo(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	w->entities.pop();
}
