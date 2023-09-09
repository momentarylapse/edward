/*
 * ActionWorldEditCamera.cpp
 *
 *  Created on: 9 Sept 2023
 *      Author: michi
 */

#include "ActionWorldEditCamera.h"

#include "../../../data/world/DataWorld.h"

ActionWorldEditCamera::ActionWorldEditCamera(int _index, const WorldCamera &c) {
	index = _index;
	camera = c;
}

void* ActionWorldEditCamera::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	std::swap(w->cameras[index], camera);
	return &w->cameras[index];
}

void ActionWorldEditCamera::undo(Data *d) {
	execute(d);
}

