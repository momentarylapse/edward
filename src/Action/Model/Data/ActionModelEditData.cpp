/*
 * ActionModelEditData.cpp
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#include "ActionModelEditData.h"

ActionModelEditData::ActionModelEditData(const DataModel::MetaData &_data)
{
	data = _data;
}

ActionModelEditData::~ActionModelEditData()
{
}

void *ActionModelEditData::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	DataModel::MetaData old_data = m->meta_data;
	m->meta_data = data;
	data = old_data;

	return NULL;
}



void ActionModelEditData::undo(Data *d)
{
	execute(d);
}


