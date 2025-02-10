/*
 * ActionWorldEditData.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "ActionWorldEditData.h"
#include <y/world/Terrain.h>

ActionWorldEditData::ActionWorldEditData(const DataWorld::MetaData &_data) {
	data = _data;
}

void ActionWorldEditData::undo(Data *d) {
	execute(d);
}



void *ActionWorldEditData::execute(Data *d) {
	auto w = dynamic_cast<DataWorld*>(d);

	std::swap(w->meta_data, data);

	return nullptr;
}


