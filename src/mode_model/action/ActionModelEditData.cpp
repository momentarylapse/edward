/*
 * ActionModelEditData.cpp
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#include "ActionModelEditData.h"
#include <algorithm>

ActionModelEditData::ActionModelEditData(const DataModel::MetaData& _data) {
	data = _data;
}

void *ActionModelEditData::execute(Data* d) {
	auto m = dynamic_cast<DataModel*>(d);

	std::swap(data, m->meta_data);

	return nullptr;
}



void ActionModelEditData::undo(Data* d) {
	execute(d);
}


