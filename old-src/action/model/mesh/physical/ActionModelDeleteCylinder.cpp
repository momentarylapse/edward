/*
 * ActionModelDeleteCylinder.cpp
 *
 *  Created on: Aug 14, 2020
 *      Author: michi
 */

#include "ActionModelDeleteCylinder.h"
#include "../../../../data/model/DataModel.h"

ActionModelDeleteCylinder::ActionModelDeleteCylinder(int _index) {
	index = _index;
}

void* ActionModelDeleteCylinder::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	cylinder = m->edit_mesh->cylinder[index];
	m->edit_mesh->cylinder.erase(index);
	return nullptr;
}

void ActionModelDeleteCylinder::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->edit_mesh->cylinder.insert(cylinder, index);
}
