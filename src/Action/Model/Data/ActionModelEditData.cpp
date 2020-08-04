/*
 * ActionModelEditData.cpp
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#include <algorithm>
#include "ActionModelEditData.h"

ActionModelEditData::ActionModelEditData(const DataModel::MetaData &_data) {
	data = _data;
}

void *ActionModelEditData::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	std::swap(data, m->meta_data);

	return NULL;
}



void ActionModelEditData::undo(Data *d) {
	execute(d);
}


