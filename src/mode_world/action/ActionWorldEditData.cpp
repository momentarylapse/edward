/*
 * ActionWorldEditData.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "ActionWorldEditData.h"
#include <lib/plugin/Instance.h>

ActionWorldEditData::ActionWorldEditData(const DataWorld::MetaData &_data) {
	data = _data;
}
void *ActionWorldEditData::execute(history::Data* d) {
	auto w = dynamic_cast<DataWorld*>(d);

	std::swap(w->meta_data, data);
	if (data.systems.num < w->meta_data.systems.num)
		w->out_system_added();
	else if (data.systems.num > w->meta_data.systems.num)
		w->out_system_removed();
	else {
		for (int i=0; i<w->meta_data.systems.num; i++)
			if (w->meta_data.systems[i] != data.systems[i])
				w->out_system_changed(i);
	}

	return nullptr;
}

void ActionWorldEditData::undo(history::Data* d) {
	execute(d);
}



